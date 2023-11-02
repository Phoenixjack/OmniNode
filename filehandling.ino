void cmd_mountfilesystem() {                                       // mount filesystem
  if (!LittleFS.begin()) {                                         //
    Serial.println("cmd_mountfilesystem: LittleFS mount failed");  //
  }                                                                //
}

void listDir(const char *dirname) {  // lists all files on internal storage to Serial Monitor only
  // usage: listDir("/");
  Serial.printf("listDir: Listing directory: %s\n", dirname);
  Dir root = LittleFS.openDir(dirname);
  while (root.next()) {
    File file = root.openFile("r");
    Serial.print("  FILE: ");
    Serial.print(root.fileName());
    Serial.print("  SIZE: ");
    Serial.print(file.size());
    time_t cr = file.getCreationTime();
    time_t lw = file.getLastWrite();
    file.close();
    struct tm *tmstruct = localtime(&cr);
    Serial.printf("    CREATION: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    tmstruct = localtime(&lw);
    Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
  }
}

String readFile(const char *path) {  // returns file contents or "error" as a String
  String strFileReadBuffer;
  Serial.printf("readFile: Reading file: %s\n", path);
  File file = LittleFS.open(path, "r");
  if (!file) {
    Serial.println("readFile: Failed to open file for reading");
    return "error";
  }
  //Serial.print("readFile: Read from file: ");
  while (file.available()) {
    strFileReadBuffer += (char)file.read();
  }
  file.close();
  return strFileReadBuffer;
}

void writeFile(const char *path, const char *filecontents) {
  // usage: writeFile("/hello.txt", "Hello ");
  Serial.printf("writeFile: Writing file: %s\n", path);
  File file = LittleFS.open(path, "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(filecontents)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  delay(2000);  // Make sure the CREATE and LASTWRITE times are different
  file.close();
}

void appendFile(const char *path, const char *filecontents) {
  // usage: appendFile("/hello.txt", "World!\n");
  Serial.printf("appendFile: Appending to file: %s\n", path);
  File file = LittleFS.open(path, "a");
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(filecontents)) {
    Serial.println("Content appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void renameFile(const char *path1, const char *path2) {
  Serial.printf("renameFile: Renaming file %s to %s\n", path1, path2);
  if (LittleFS.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void deleteFile(const char *path) {
  // usage: deleteFile("/hello.txt");
  Serial.printf("deleteFile: Deleting file: %s\n", path);
  if (LittleFS.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void cmd_unmountfilesystem() {  // unmount filesystem
  LittleFS.end();               //
}

void cmd_wipefilesystem() {  // format the internal storage
  LittleFS.format();         //
}