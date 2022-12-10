# Diary

Diary is a command-line app for keeping a diary.

It saves the data in a file called ~/.diary.

# Usage

Write a new diary entry: 

```
echo "Lorem ipsum dolor sit amet." | diary write
```

Read the diary:

```
diary read
```

It can be helpful to pipe the diary to `less`, so you can search it. Like this:

```
diary read | less
```

To search, type `/`, then the search query, then press Enter. You can step through the matches by pressing `n`.

# Installation

You need a C compiler. I used GCC 11.3.0. I have only tested this installation on Ubuntu Linux.

Clone the repository:

```
git clone git@github.com:8n8/diary.git
```

Change directory into the repository:

```
cd diary
```

Build the executable:

```
gcc -O2 main.c -o diary
```

Move the executable to somewhere on the path:

```
mv diary ~/.local/bin
```


# Data format

You only need to read this if you want to write your own parser for the data file.

Each entry has a unique ID, a timestamp, and a piece of text.

## Unique ID

The unique ID is a non-negative integer, and is the position of the entry in the data file, so the first item has ID 0, the second has ID 1 and so on. So this is implied by the position in the file and is not explicitly recorded.

## Timestamp

The timestamp is the number of minutes since Unix time 1670449116, which is the time of writing. It is 32 bits long, so can store 2^32 minutes ~= 8172 years. Timestamps before 1670449116 Unix time are not supported.

Encoding is little-endian.

## Text

The piece of text is UTF-8 encoded. It is prefixed by a 16-bit length, little-endian encoded. So the maximum size of any diary entry is 2^16 - 1 bytes, or about 65kB.
