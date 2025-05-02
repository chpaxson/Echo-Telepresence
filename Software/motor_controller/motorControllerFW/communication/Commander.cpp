// Ported Commander.cpp for Raspberry Pi Pico SDK (C++)

#include "Commander.h"

Commander::Commander(char eol, bool echo) {
  this->eol = eol;
  this->echo = echo;
}

void Commander::add(char id, CommandCallback onCommand, const char* label) {
  call_list[call_count] = onCommand;
  call_ids[call_count] = id;
  call_label[call_count] = (char*)label;
  call_count++;
}

void Commander::run() {
  int ch;
  while ((ch = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT) {
    received_chars[rec_cnt++] = (char)ch;
    if (echo) putchar(ch);

    if (isSentinel(ch)) {
      received_chars[rec_cnt] = '\0';
      run(received_chars);
      rec_cnt = 0;
    }
    if (rec_cnt >= MAX_COMMAND_LENGTH) {
      rec_cnt = 0;
    }
  }
}

void Commander::run(char* user_input) {
  char id = user_input[0];
  switch(id) {
    case CMD_SCAN:
      for(int i = 0; i < call_count; i++) {
        printMachineReadable(CMD_SCAN);
        print(call_ids[i]);
        print(":");
        if(call_label[i]) println(call_label[i]);
        else println("");
      }
      break;
    case CMD_VERBOSE:
      if(!isSentinel(user_input[1])) verbose = (VerboseMode)atoi(&user_input[1]);
      printVerbose("Verb:");
      printMachineReadable(CMD_VERBOSE);
      switch(verbose) {
        case VerboseMode::nothing:
          println("off!"); break;
        case VerboseMode::on_request:
        case VerboseMode::user_friendly:
          println("on!"); break;
        case VerboseMode::machine_readable:
          printlnMachineReadable("machine"); break;
      }
      break;
    case CMD_DECIMAL:
      if(!isSentinel(user_input[1])) decimal_places = atoi(&user_input[1]);
      printVerbose("Decimal:");
      printMachineReadable(CMD_DECIMAL);
      println(decimal_places);
      break;
    default:
      for(int i = 0; i < call_count; i++) {
        if(id == call_ids[i]) {
          printMachineReadable(user_input[0]);
          call_list[i](&user_input[1]);
          break;
        }
      }
      break;
  }
}

void Commander::print(const int number) {
  if(verbose == VerboseMode::nothing) return;
  printf("%d", number);
}

void Commander::print(const float number) {
  if(verbose == VerboseMode::nothing) return;
  printf("%.*f", decimal_places, number);
}

void Commander::print(const char* message) {
  if(verbose == VerboseMode::nothing) return;
  printf("%s", message);
}

void Commander::print(const char message) {
  if(verbose == VerboseMode::nothing) return;
  putchar(message);
}

void Commander::println(const int number) {
  if(verbose == VerboseMode::nothing) return;
  printf("%d\n", number);
}

void Commander::println(const float number) {
  if(verbose == VerboseMode::nothing) return;
  printf("%.*f\n", decimal_places, number);
}

void Commander::println(const char* message) {
  if(verbose == VerboseMode::nothing) return;
  printf("%s\n", message);
}

void Commander::println(const char message) {
  if(verbose == VerboseMode::nothing) return;
  printf("%c\n", message);
}

void Commander::printVerbose(const char* message) {
  if(verbose == VerboseMode::user_friendly) print(message);
}

void Commander::printMachineReadable(const int number) {
  if(verbose == VerboseMode::machine_readable) print(number);
}

void Commander::printMachineReadable(const float number) {
  if(verbose == VerboseMode::machine_readable) print(number);
}

void Commander::printMachineReadable(const char* message) {
  if(verbose == VerboseMode::machine_readable) print(message);
}

void Commander::printMachineReadable(const char message) {
  if(verbose == VerboseMode::machine_readable) print(message);
}

void Commander::printlnMachineReadable(const int number) {
  if(verbose == VerboseMode::machine_readable) println(number);
}

void Commander::printlnMachineReadable(const float number) {
  if(verbose == VerboseMode::machine_readable) println(number);
}

void Commander::printlnMachineReadable(const char* message) {
  if(verbose == VerboseMode::machine_readable) println(message);
}

void Commander::printlnMachineReadable(const char message) {
  if(verbose == VerboseMode::machine_readable) println(message);
}

void Commander::printError() {
  println("err");
}

bool Commander::isSentinel(char ch) {
  return ch == eol || ch == '\r';
}
