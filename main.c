#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <wordexp.h>

#define TEXT_BUF_SIZE (256 * 256 - 1)
#define EPOCH 1670449116

uint8_t textBuf[TEXT_BUF_SIZE];

static void get_timestamp(uint8_t timestamp[4]) {
	int seconds = (int)time(NULL) - EPOCH;
	int minutes = seconds / 60;
	timestamp[0] = minutes & 0xFF;
	timestamp[1] = (minutes >> 8) & 0xFF;
	timestamp[2] = (minutes >> 16) & 0xFF;
	timestamp[3] = (minutes >> 24) & 0xFF;
}

static void encode_size(size_t size, uint8_t encoded[2]) {
	encoded[0] = size & 0xFF;
	encoded[1] = (size >> 8) & 0xFF;
}

static int is_valid_utf8(uint8_t buf[TEXT_BUF_SIZE], size_t size) {
	int byte_num = 1;
	for (size_t i = 0; i < size; ++i) {
		if (byte_num == 1 && (buf[i] >> 7) == 0) {
			continue;
		}

		if (byte_num == 1 && (buf[i] >> 5) == 6) {
			byte_num == 2;
			continue;
		}

		if (byte_num == 2 && (buf[i] >> 6) == 2) {
			byte_num = 1;
			continue;
		}

		if (byte_num == 1 && (buf[i] >> 4) == 14) {
			byte_num = 2;
			continue;
		}

		if (byte_num == 2 && (buf[i] >> 6) == 2) {
			byte_num = 3;
			continue;
		}

		if (byte_num == 3 && (buf[i] >> 6) == 2) {
			byte_num = 1;
			continue;
		}

		if (byte_num == 1 && (buf[i] >> 3) == 30) {
			byte_num = 2;
			continue;
		}

		if (byte_num == 2 && (buf[i] >> 6) == 2) {
			byte_num = 3;
			continue;
		}

		if (byte_num == 3 && (buf[i] >> 6) == 2) {
			byte_num = 4;
			continue;
		}

		if (byte_num == 4 && (buf[i] >> 6) == 2) {
			byte_num = 1;
			continue;
		}

		return 0;
	}

	return 1;
}

static char* diary_path() {
	wordexp_t result;
	wordexp("~/.diary", &result, 0);
	return result.we_wordv[0];
}

static int write_diary() {
	size_t size = fread(textBuf, 1, TEXT_BUF_SIZE, stdin);
	if (!feof(stdin)) {
		fputs("entry was greater than 65kB\n", stderr);
		return -1;
	}
	if (size == 0) {
		fputs("empty entry\n", stderr);
		return -1;
	}

	if (!is_valid_utf8(textBuf, size)) {
		fputs("invalid text\n", stderr);
		return -1;
	}

	uint8_t timestamp[4];
	get_timestamp(timestamp);

	FILE* file = fopen(diary_path(), "ab");
	if (file == NULL) {
		fputs("failed to save: could not open ~/.diary\n", stderr);
		return -1;
	}

	size_t written = fwrite(timestamp, 1, 4, file); 
	if (written != 4) {
		fputs("could not save timestamp\n", stderr);
		return -1;
	}

	uint8_t encoded_size[2];
	encode_size(size, encoded_size);

	written = fwrite(encoded_size, 1, 2, file);
	if (written != 2) {
		fputs("could not save entry size\n", stderr);
		return -1;
	}

	written = fwrite(textBuf, 1, size, file);
	if (written != size) {
		fputs("could not save diary entry\n", stderr);
		return -1;
	}

	return 0;
}


int parse_uint32(uint8_t buf[4]) {
	return buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24);
}

int parse_uint16(uint8_t buf[2]) {
    return buf[0] + (buf[1] << 8);
}


void print_timestamp(int minutes) {
	time_t posix_seconds = (minutes * 60) + EPOCH;
	struct tm* local = localtime(&posix_seconds);

	char buf[36] = "Wednesday 31 September 2022 2:30pm\0";
	strftime(buf, 36, "%A %-e %B %Y %-I:%M %p", local);
	printf("%s\n\n", buf);
}


static int read_diary() {
	FILE* file = fopen(diary_path(), "rb");
	if (file == NULL) {
		fputs("failed to save: could not open ~/.diary\n", stderr);
		return -1;
	}

	uint8_t time_buf[4];
	uint8_t size_buf[2];
	int first_time = 1;

	while (1) {
		size_t size = fread(time_buf, 1, 4, file);
		if (feof(file)) {
			return 0;
		}
		if (size != 4) {
			fprintf(stderr, "corrupted file: timestamp contained %ld bytes but should contain 4\n", size);
			return -1;
		}

		if (first_time) {
			first_time = 0;
		} else {
			fputs("\n\n", stdout);
		}

		print_timestamp(parse_uint32(time_buf));

		size = fread(size_buf, 1, 2, file);
		if (size != 2) {
			fprintf(stderr, "corrupted file: text length contained %ld bytes but should contain 2\n", size);
			return -1;
		}

		int text_size = parse_uint16(size_buf);

		size = fread(textBuf, 1, text_size, file);
		if (size != text_size) {
			fprintf(stderr, "corrupted file: expecting %d bytes of text, but got %ld\n", text_size, size);
			return -1;
		}

		for (int i = 0; i < text_size; ++i) {
			putchar(textBuf[i]);
		}
	}

	return 0;
}

char* usage = "invalid arguments: usage instructions are at https://github.com/8n8/diary\n";

static int is_write(char* arg) {
	return arg[0] == 'w' && arg[1] == 'r' && arg[2] == 'i' && arg[3] == 't' && arg[4] == 'e' && arg[5] == '\0';
}

static int is_read(char* arg) {
	return arg[0] == 'r' && arg[1] == 'e' && arg[2] == 'a' && arg[3] == 'd' && arg[4] == '\0';
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		fputs(usage, stderr);
		return -1;
	}

	if (is_write(argv[1])) {
		return write_diary();
	}

	if (is_read(argv[1])) {
		return read_diary();
	}

	fputs(usage, stderr);
	return -1;
}
