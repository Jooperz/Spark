CC := clang
CFLAGS := -g -Wall -Wno-deprecated-declarations -Werror
all: spark

clean:
	rm -f spark

spark: spark.c 
	$(CC) $(CFLAGS) -o spark spark.c message.c util.c -lform -lncurses -lpthread

zip:
	@echo "Generating spark.zip file to submit to Gradescope..."
	@zip -q -r spark.zip . -x .git/\* .vscode/\* .clang-format .gitignore spark
	@echo "Done. Please upload spark.zip to Gradescope."

format:
	@echo "Reformatting source code."
	@clang-format -i --style=file $(wildcard *.c) $(wildcard *.h)
	@echo "Done."

.PHONY: all clean zip format
