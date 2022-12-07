# Diary

** This is a work in progress and isn't ready to use yet. **

Diary is a command-line app for recording diary entries.

It saves the data in a file called ~/.diary.

# Usage

Record a new diary entry: 

```
echo "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum." | diary
```

# Data format

Each entry has a unique ID, a timestamp, and a piece of text.

## Unique ID

The unique ID is a non-negative integer, and is the position of the entry in the data file, so the first item has ID 0, the second has ID 1 and so on.

## Timestamp

The timestamp is the number of minutes since Unix time 1670449116, which is the time of writing. It is 32 bits long, so can store 2^32 minutes ~= 8172 years. Timestamps before 1670449116 Unix time are not supported.

Encoding is little-endian.

## Text

The piece of text is UTF-8 encoded. It is prefixed by a 16-bit length, little-endian encoded. So the maximum size of any diary entry is 2^16 bytes, or about 65kB.
