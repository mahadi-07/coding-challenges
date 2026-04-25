#!/usr/bin/env bash

is_pipe=false
if [ -p /dev/stdin ]; then
  is_pipe=true
fi

# No input
if [ "$#" -eq 0 ] && [ "$is_pipe" = false ]; then
  echo "Try this: ccwc <option> <filename>"
  exit 1
fi

# pipe input
input_buffer=$(mktemp)
trap 'rm -f "$input_buffer"' EXIT # always delete the file stored in input_buffer.

if [ "$is_pipe" = true ]; then
  cat > "$input_buffer"
fi

# Flags
count_bytes=false
count_lines=false
count_characters=false
count_words=false

for arg in "$@"; do
  case "$arg" in
    -c) count_bytes=true ;;
    -l) count_lines=true ;;
    -w) count_words=true ;;
    -m) count_characters=true ;;
    *)
      if [[ "$arg" == *.txt ]]; then

        if [ ! -f "$arg" ]; then
          echo "File does not exist: $arg"
          exit 1
        fi

        file="$arg"
        cat "$arg" > "$input_buffer"
      else
        echo "Error: only .txt files are supported"
        exit 1
      fi
      ;;
  esac
done

# echo "count_bytes=$count_bytes"
# echo "count_lines=$count_lines"
# echo "count_words=$count_words"
# echo "count_characters=$count_characters"
# cat "$input_buffer"

if [ -n "$file" ]; then
  cat "$file" > "$input_buffer"
fi

# Counting
lines=0
words=0
bytes=0
chars=0
in_word=false

while IFS= read -r line || [[ -n "$line" ]]; do
  lines=$((lines + 1))
done < "$input_buffer"

while IFS= read -r -n1 char || [[ -n "$char" ]]; do
  # byte + char count (for most cases they match in ASCII files)
  bytes=$((bytes + 1))
  chars=$((chars + 1))

  # word count logic
  if [[ "$char" =~ [[:space:]] ]]; then
    in_word=false
  else
    if [[ "$in_word" == false ]]; then
      words=$((words + 1))
      in_word=true
    fi
  fi

done < "$input_buffer"

output=""

if [ "$count_lines" = true ]; then
  output="$output$(printf "%8d" "$lines")"
fi

if [ "$count_bytes" = true ]; then
  output="$output$(printf "%8d" "$bytes")"
fi

if [ "$count_words" = true ]; then
  output="$output$(printf "%8d" "$words")"
fi

if [ "$count_characters" = true ]; then
  output="$output$(printf "%8d" "$chars")"
fi

# default behavior (no flags)
if [ -z "$output" ]; then
  printf "%8d %8d %8d %s\n" "$lines" "$words" "$bytes" "$file"
else
  printf "%s %s\n" "$output" "$file"
fi