#!/usr/bin/env bash
#
# Updates the vendored float_tools headers to the latest version from upstream.
# https://github.com/jorgen/float_tools
#
set -euo pipefail

REPO_URL="https://github.com/jorgen/float_tools.git"
DEST_DIR="$(dirname "$0")/include/structify/float_tools"
TMPDIR="$(mktemp -d)"

trap 'rm -rf "$TMPDIR"' EXIT

echo "Cloning float_tools from $REPO_URL ..."
git clone --depth 1 "$REPO_URL" "$TMPDIR/float_tools"

SHA=$(git -C "$TMPDIR/float_tools" rev-parse HEAD)
DATE=$(git -C "$TMPDIR/float_tools" log -1 --format=%ci)
MSG=$(git -C "$TMPDIR/float_tools" log -1 --format=%s)

echo "Latest commit: $SHA"
echo "Date:          $DATE"
echo "Message:       $MSG"

echo "Copying headers to $DEST_DIR ..."
cp "$TMPDIR/float_tools/include/float_tools.h" "$DEST_DIR/"
cp "$TMPDIR/float_tools/include/cache_ryu.h" "$DEST_DIR/"

echo ""
echo "Updated float_tools headers to $SHA"
echo "Remember to commit the updated headers."
