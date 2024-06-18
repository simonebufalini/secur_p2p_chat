#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

char * readFile(const char * filename) {
  FILE * file;
  long fileSize;
  char * buffer;

  // Open the file for reading
  file = fopen(filename, "rb");
  if (file == NULL) {
    printf("Unable to open file.\n");
    return NULL;
  }

  // Determine the file size
  fseek(file, 0, SEEK_END);
  fileSize = ftell(file);
  rewind(file);

  // Allocate memory to store the file content
  buffer = (char * ) malloc(fileSize + 1);
  if (buffer == NULL) {
    printf("Memory allocation failed.\n");
    fclose(file);
    return NULL;
  }

  // Read the file content into the buffer
  if (fread(buffer, 1, fileSize, file) != fileSize) {
    printf("Error reading file.\n");
    fclose(file);
    free(buffer);
    return NULL;
  }

  // Null-terminate the buffer
  buffer[fileSize] = '\0';

  // Close the file
  fclose(file);

  return buffer;

}
