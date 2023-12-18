void cmd_mountfilesystem() {                 // mount filesystem
  if (!LittleFS.begin()) {                   //
    debugprefix();                           //
    debugoutput("LittleFS mount failed\n");  //
  }                                          //
}

void listDir(const char *dirname) {  // lists all files on internal storage to Serial Monitor only [for now]
#if (defDebugFull)
  // usage: listDir("/");
  debugprefix();  //
  debugoutput("Listing directory: ");
  debugoutputln(dirname);
  Dir root = LittleFS.openDir(dirname);
  while (root.next()) {
    File file = root.openFile("r");
    debugoutput("  FILE: ");
    debugoutput(root.fileName());
    debugoutput("  SIZE: ");
    debugoutput(file.size());
    time_t cr = file.getCreationTime();
    time_t lw = file.getLastWrite();
    file.close();
    struct tm *tmstruct = localtime(&cr);
    // debugoutput("    CREATION: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    debugoutput("    CREATION: ");
    debugoutput((tmstruct->tm_year) + 1900);
    debugoutput((tmstruct->tm_mon) + 1);
    debugoutput(tmstruct->tm_mday);
    debugoutput(tmstruct->tm_hour);
    debugoutput(tmstruct->tm_min);
    debugoutput(tmstruct->tm_sec);
    tmstruct = localtime(&lw);
    //debugoutput("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
  }
#endif
}

String readFile(const char *path) {  // returns file contents or "error" as a String
  String strFileReadBuffer;
  debugprefix();  //
  debugoutput("Reading file: ");
  debugoutputln(path);
  File file = LittleFS.open(path, "r");
  if (!file) {
    debugoutput("Failed to open file for reading\n");
    return "error";
  }
  //debugoutput("Read from file: ");
  while (file.available()) {
    strFileReadBuffer += (char)file.read();
  }
  file.close();
  return strFileReadBuffer;
}

void writeFile(const char *path, const char *filecontents) {
  // usage: writeFile("/hello.txt", "Hello ");
  debugprefix();  //
  debugoutput("writeFile: Writing file: ");
  debugoutputln(path);
  File file = LittleFS.open(path, "w");
  if (!file) {
    debugoutput("Failed to open file for writing\n");
    return;
  }
  if (file.print(filecontents)) {
    debugoutput("File written\n");
  } else {
    debugoutput("Write failed\n");
  }
  delay(2000);  // Make sure the CREATE and LASTWRITE times are different
  file.close();
}

void appendFile(const char *path, const char *filecontents) {
  // usage: appendFile("/hello.txt", "World!\n");
  debugprefix();  //
  debugoutput("Appending to file: ");
  debugoutputln(path);
  File file = LittleFS.open(path, "a");
  if (!file) {
    debugoutput("Failed to open file for appending\n");
    return;
  }
  if (file.print(filecontents)) {
    debugoutput("Content appended\n");
  } else {
    debugoutput("Append failed\n");
  }
  file.close();
}

void renameFile(const char *path1, const char *path2) {
  debugprefix();  //
  debugoutput("Renaming file: ");
  debugoutput(path1);
  debugoutput(" to ");
  debugoutputln(path2);
  if (LittleFS.rename(path1, path2)) {
    debugoutput("File renamed\n");
  } else {
    debugoutput("Rename failed\n");
  }
}

void deleteFile(const char *path) {
  // usage: deleteFile("/hello.txt");
  debugprefix();  //
  debugoutput("Deleting file: ");
  debugoutputln(path);
  if (LittleFS.remove(path)) {
    debugoutput("File deleted\n");
  } else {
    debugoutput("Delete failed\n");
  }
}

void cmd_unmountfilesystem() {  // unmount filesystem
  LittleFS.end();               //
}

void cmd_wipefilesystem() {  // format the internal storage
  LittleFS.format();         //
}