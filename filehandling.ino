void cmd_mountfilesystem() {             // mount filesystem
  if (!LittleFS.begin()) {               //
    debugPrefix(defDebugFileSys);    //
    debugFS("LittleFS mount failed\n");  //
  }                                      //
}

String listDir(const char *dirname) {  // lists all files on internal storage. Can print or return list as string
                                       // usage: listDir("/");
  String strFileList = "";
  debugPrefix(defDebugFileSys);    //
  strFileList += "Listing directory: ";
  strFileList += dirname;
  strFileList += "\n\n";
  Dir root = LittleFS.openDir(dirname);
  while (root.next()) {
    File file = root.openFile("r");
    strFileList += "  FILE: ";
    strFileList += root.fileName();
    strFileList += "  SIZE: ";
    strFileList += file.size();
    time_t cr = file.getCreationTime();
    time_t lw = file.getLastWrite();
    file.close();
    struct tm *tmstruct = localtime(&cr);
    // debugoutput("    CREATION: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    strFileList += "    CREATION: ";
    strFileList += (tmstruct->tm_year) + 1900;
    strFileList += (tmstruct->tm_mon) + 1;
    strFileList += tmstruct->tm_mday;
    strFileList += tmstruct->tm_hour;
    strFileList += tmstruct->tm_min;
    strFileList += tmstruct->tm_sec;
    tmstruct = localtime(&lw);
    debugFS("\n");
    //debugoutput("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
  }
  debugFS(strFileList);
  return strFileList;
}

String readFile(const char *path) {  // returns file contents or "error" as a String
  String strFileReadBuffer;
  debugPrefix(defDebugFileSys);    //
  debugFS("Reading file: ");
  debugFS(path);
  File file = LittleFS.open(path, "r");
  if (!file) {
    debugFS("... FAILED TO OPEN\n");
    return "error";
  }
  while (file.available()) {
    strFileReadBuffer += (char)file.read();
  }
  file.close();
  debugFS("... OPENED\n");
  debugFS(strFileReadBuffer);
  debugFS("\n");
  return strFileReadBuffer;
}

void writeFile(const char *path, const char *filecontents) {  // usage: writeFile("/hello.txt", "Hello ");
  debugPrefix(defDebugFileSys);    //
  debugFS("Writing to ");
  debugFS(path);
  debugFS(": ");
  File file = LittleFS.open(path, "w");
  if (!file) {
    debugFS("... FAILED TO OPEN\n");
    return;
  }
  if (file.print(filecontents)) {
    debugFS("... SUCCESS\n");
  } else {
    debugFS("... FAILED\n");
  }
  delay(2000);  // Make sure the CREATE and LASTWRITE times are different
  file.close();
}

void appendFile(const char *path, const char *filecontents) {  // usage: appendFile("/hello.txt", "World!\n");
  debugPrefix(defDebugFileSys);    //
  debugFS("Appending to file: ");
  debugFS(path);
  File file = LittleFS.open(path, "a");
  if (!file) {
    debugFS("... FAILED TO OPEN\n");
    return;
  }
  if (file.print(filecontents)) {
    debugFS("... SUCCESS\n");
  } else {
    debugFS("... FAILED\n");
  }
  file.close();
}

void renameFile(const char *path1, const char *path2) {
  debugPrefix(defDebugFileSys);    //
  debugFS("Renaming file: ");
  debugFS(path1);
  debugFS(" to ");
  debugFS(path2);
  if (LittleFS.rename(path1, path2)) {
    debugFS("... SUCCESS\n");
  } else {
    debugFS("... FAILED\n");
  }
}

void deleteFile(const char *path) {  // usage: deleteFile("/hello.txt");
  debugPrefix(defDebugFileSys);    //
  debugFS("Deleting file: ");
  debugFS(path);
  if (LittleFS.remove(path)) {
    debugFS("... SUCCESS\n");
  } else {
    debugFS("... FAILED\n");
  }
}

void cmd_unmountfilesystem() {  // unmount filesystem
  LittleFS.end();               //
}

void cmd_wipefilesystem() {  // format the internal storage
  LittleFS.format();         //
}