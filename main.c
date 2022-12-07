#define TEXT_BUF_SIZE 256 * 256

uint8_t textBuf[TEXT_BUF_SIZE];


int main(int argc, char* argv[]) {
	FILE* file = fopen("~/.diary", "ab");
	if (file == NULL) {
		fputs("failed to save\n", stderr);
		return -1;
	}

	size_t size = fread(textBuf, 1, TEXT_BUF_SIZE, stdin);
	if (!feof(stdin)) {
		fputs("entry was greater than 65kB\n");
		return -1;
	}

	if (size == 0) {
		fputs("empty entry\n");
		return -1;
	}
}
