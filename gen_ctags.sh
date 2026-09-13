#!/usr/bin/env bash
# gen-tags.sh —— 项目 tags + 按 include 自动生成的系统库 tags
set -e
cd "$(dirname "$0")"
ctags -f .ctags -R .
SRCS=$(find . -name '*.c' | grep -v build)   # 按需调整
gcc -M $SRCS 2>/dev/null | tr ' \\' '\n' | grep '\.h$' \
  | grep -v "$PWD/" | grep -vE '/(bits|sys|asm|gnu)/' | sort -u \
  | xargs ctags --quiet --c++-kinds=+p -f .tags.sys
echo "tags 已生成: $(wc -l < .ctags) + $(wc -l < .tags.sys)"
