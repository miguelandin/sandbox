#!/usr/bin/env bash
# Integration tests for sandbox.
# Every "virus" in tests/src tries exactly one forbidden action and prints
# ESCAPED if it succeeded or CONTAINED if it was stopped. Each case is run
# without the permission flag (must be contained) and, when it makes sense,
# with it (must escape), so a broken virus cannot pass by accident.
#
# Usage: ./tests/run.sh            (from the repository root, or via make test)
#        SHOW_STDERR=1 ./tests/run.sh   to see sandbox's own errors

cd "$(dirname "$0")/.." || exit 1

SANDBOX=./bin/sandbox
T=./tests/bin
MARKER=/tmp/sandbox_test_marker
SECRET_TEXT="TOP_SECRET_$RANDOM"

pass=0; fail=0; skip=0
LOGS=()

green=$'\033[1;32m'; red=$'\033[1;31m'; yellow=$'\033[1;33m'; reset=$'\033[0m'
[ -t 1 ] || { green=; red=; yellow=; reset=; }

ok()   { echo "${green}PASS${reset}  $1"; pass=$((pass + 1)); }
bad()  { echo "${red}FAIL${reset}  $1"; fail=$((fail + 1)); }
skip() { echo "${yellow}SKIP${reset}  $1"; skip=$((skip + 1)); }

new_log() {
  local log
  log=$(mktemp /tmp/sandbox_test_log.XXXXXX)
  LOGS+=("$log")
  echo "$log"
}

# run <log> <sandbox args...>  -> prints the binary's stdout
run() {
  local log=$1; shift
  if [ -n "$SHOW_STDERR" ]; then
    "$SANDBOX" -s -o "$log" "$@"
  else
    "$SANDBOX" -s -o "$log" "$@" 2>/dev/null
  fi
}

# expect <name> <word> <log> <sandbox args...>
expect() {
  local name=$1 want=$2 log=$3 out; shift 3
  out=$(run "$log" "$@")
  if [[ "$out" == *"$want"* ]]; then
    ok "$name"
  else
    bad "$name (expected $want, got: ${out:-<nothing>})"
  fi
}

# jq -e exit codes: 0 = some line matched, 4 = nothing matched,
# anything else = the log is not valid JSON
query() { jq -e "select($2)" "$1" >/dev/null 2>&1; echo $?; }

# logged <name> <log> <jq filter>  -> passes if at least one line matches
logged() {
  case $(query "$2" "$3") in
    0) ok "$1" ;;
    4) bad "$1 (no matching event in log)" ;;
    *) bad "$1 (log is not valid JSON)" ;;
  esac
}

# not_logged <name> <log> <jq filter>  -> passes if no line matches
not_logged() {
  case $(query "$2" "$3") in
    4) ok "$1" ;;
    0) bad "$1 (unexpected event in log)" ;;
    *) bad "$1 (log is not valid JSON)" ;;
  esac
}

cleanup() {
  rm -f "$MARKER" "${SECRET_FILE:-}" "${LOGS[@]}"
  rm -f /tmp/x\"}*
}
trap cleanup EXIT

# --- preconditions -----------------------------------------------------------
for dep in "$SANDBOX" jq; do
  command -v "$dep" >/dev/null 2>&1 || [ -x "$dep" ] || {
    echo "missing dependency: $dep"; exit 1; }
done

echo "== launch =="
log=$(new_log)
expect "script inside /home is launched" HELLO "$log" "$T/launch_script.sh"
not_logged "reads of the launched script are not reported as blocked" "$log" \
  '(.target | endswith("launch_script.sh")) and .allowed == false'

echo "== write =="
rm -f "$MARKER"
log=$(new_log)
expect "write blocked without -w" CONTAINED "$log" "$T/write_file"
[ ! -e "$MARKER" ] && ok "no file created on the host" \
                   || bad "file was created on the host"
logged "blocked write is logged" "$log" \
  '(.group == "write" or .group == "read/write") and .allowed == false'

rm -f "$MARKER"
expect "write allowed with -w" ESCAPED "$(new_log)" -w "$T/write_file"
[ -e "$MARKER" ] && ok "file created on the host with -w" \
                 || bad "file not created with -w"
rm -f "$MARKER"

echo "== read =="
case "$HOME" in
  /home/*|/root|/root/*)
    SECRET_FILE="$HOME/.sandbox_test_secret"
    echo "$SECRET_TEXT" > "$SECRET_FILE"

    log=$(new_log)
    out=$(run "$log" "$T/read_home" "$SECRET_FILE")
    [[ "$out" == *CONTAINED* ]] && ok "home read blocked without -r" \
                                || bad "home read not blocked (got: $out)"
    [[ "$out" != *"$SECRET_TEXT"* ]] && ok "secret not leaked" \
                                     || bad "SECRET LEAKED"
    logged "blocked home read is logged" "$log" \
      '(.target | endswith(".sandbox_test_secret")) and .allowed == false'

    out=$(run "$(new_log)" -r "$T/read_home" "$SECRET_FILE")
    [[ "$out" == *"$SECRET_TEXT"* ]] && ok "home read allowed with -r" \
                                     || bad "home read failed with -r (got: $out)"
    ;;
  *)
    skip "read tests: \$HOME ($HOME) is not a protected directory"
    ;;
esac

echo "== exec =="
log=$(new_log)
expect "exec blocked without -x" CONTAINED "$log" "$T/exec_child"
logged "blocked exec is logged" "$log" \
  '.group == "execution" and .allowed == false'
expect "exec allowed with -x" ESCAPED "$(new_log)" -x "$T/exec_child"

echo "== network =="
log=$(new_log)
expect "connect blocked without -n" CONTAINED "$log" "$T/net_connect"
logged "blocked connect is logged" "$log" \
  '.group == "network" and .allowed == false'
if timeout 3 bash -c 'exec 3<>/dev/tcp/1.1.1.1/80' 2>/dev/null; then
  expect "connect allowed with -n" ESCAPED "$(new_log)" -n "$T/net_connect"
else
  skip "connect with -n: no internet access on this machine"
fi

echo "== attacks against sandbox =="
expect "log file descriptor is not inherited" CONTAINED "$(new_log)" "$T/fd_leak"

log=$(new_log)
run "$log" "$T/evil_name" >/dev/null
if jq empty "$log" >/dev/null 2>&1; then
  ok "log is valid JSON after a malicious file name"
else
  bad "log is not valid JSON after a malicious file name"
fi
not_logged "no fake event injected into the log" "$log" \
  '.target == "evil.com:443"'

# --- summary -----------------------------------------------------------------
echo
echo "$pass passed, $fail failed, $skip skipped"
[ "$fail" -eq 0 ]
