// I write shit. I apologise.

#include<stdlib.h>
#include<stdio.h>
#include<windef.h>
#include<winuser.h>
#include<getopt.h>


// If you want to change the follow macros to get the programme used in other purpose,
// use compiler options, e.g., in g++, -DXXX=xxx, such as -DWNAME_DEFAULT=\"foo\".

#ifndef WNAME_SIZE
#define WNAME_SIZE 0x100
#endif

#ifndef MSG_SIZE
#define MSG_SIZE 0x10000
#endif

#ifndef WNAME_DEFAULT
#define WNAME_DEFAULT "AliceInCradle"
#endif

#ifndef DWDATA_DEFAULT
#define DWDATA_DEFAULT 0x6d756b6f75616f69ll
#endif


union Handle {
    HWND hwnd;
    int value;
};


enum OPT {
    OPT_NULL    = 0x00,
    OPT_HELP    = 0x01,
    OPT_VERSION = 0x02,
    OPT_QUIET   = 0x04
};

OPT operator|=(OPT &opt, OPT opt1) {
    return opt = (OPT)(opt | opt1);
}


void GetOptionsAndHandle(int argc, char *argv[], OPT &options, Handle &handle, long long &data);
bool PrintInfo(OPT options);
Handle InputHandle(OPT options);
bool InputMsg(OPT options, char message[], rsize_t msgsize);
void SendMsg(Handle handle, long long dwData, char message[], rsize_t msgsize);

int main(int argc, char *argv[]) {

    OPT options = OPT_NULL; Handle handle = { nullptr }; long long dwData = DWDATA_DEFAULT;
    GetOptionsAndHandle(argc, argv, options, handle, dwData);

    if(PrintInfo(options)) return 0;

    if(handle.hwnd == nullptr) handle = InputHandle(options);
    if(!(options & OPT_QUIET)) printf_s("Handle: %08x\n", handle.value);

    char message[MSG_SIZE];
    while(InputMsg(options, message, sizeof(message))) {
        SendMsg(handle, dwData, message, sizeof(message));
    }

    return 0;
}


void GetOptionsAndHandle(int argc, char *argv[], OPT &options, Handle &handle, long long &data) {
    static const char opts[] = "hvqH:N:C:d:";
    static const option optslong[] = {
        { "help",       no_argument,        nullptr,    'h' },
        { "version",    no_argument,        nullptr,    'v' },
        { "quiet",      no_argument,        nullptr,    'q' },
        { "handle",     required_argument,  nullptr,    'H' },
        { "name",       required_argument,  nullptr,    'N' },
        { "class",      required_argument,  nullptr,    'C' },
        { "data",       required_argument,  nullptr,    'd' }
    };

    char opt;
    while((opt = getopt_long(argc, argv, opts, optslong, nullptr)) != EOF) {
        switch(opt) {
        case 'h':
            options |= OPT_HELP;
            break;
        case 'v':
            options |= OPT_VERSION;
            break;
        case 'q':
            options |= OPT_QUIET;
            break;
        case 'H': case 'N': case 'C':
            if(handle.hwnd != nullptr){
                fputs("are-client: Handle has already set.\n", stderr);
                break;
            }
            switch(opt){
            case 'H':
                handle.value = atoi(optarg);
                break;
            case 'N':
                handle.hwnd = FindWindowA(nullptr, optarg);
                break;
            case 'C':
                handle.hwnd = FindWindowA(optarg, nullptr);
                break;
            }
            break;
        case 'd':
            data = atoll(optarg);
            break;
        }
    }
}


bool PrintInfo(OPT options) {
    static const char help[] = 
        "are-client.exe\n\n"
        "Send string messages to other processes.\n\n"
        "Usage: are-client.exe [ [ -H <HANDLE> | -N <WINDOW_NAME> | -C <CLASS_NAME> ] [-p <PARAM>] [-q] | -h | -v ]\n\n"
        "You need to assign a handle of a window before sending message to the window.\n\n"
        "Options:\n"
        "    -H, --handle    Assign the handle (by giving a int).\n"
        "    -N, --name      Assign the handle by giving the window's name.\n"
        "    -C, --class     Assign the handle by giving the class name.\n"
        "    -d, --data      Additional information(int64) you want to send to the window.\n"
        "    -q, --quiet     Run in the quiet mode: no output and read until EOF(^Z).\n"
        "    -h, --help      Print this help message.\n"
        "    -v, --version   Print the version information.\n\n"
        "Console Mode:\n\n"
        "If this programme is not running in the quiet mode, then it is running in the console mode.\n\n"
        "In console mode, you can write your message after '$', and press enter to send it.\n"
        "Example:\n"
        "    $FOO\n"
        "Actual message:\n"
        "    FOO\n\n"
        "If the brackets are not closed, there will be new lines starting with '.' for you to continue your message.\n"
        "Example:\n"
        "    $IF bar {\n"
        "    .    FOO\n"
        "    .}\n"
        "Actual message:\n"
        "    IF bar {\n"
        "        FOO\n"
        "    }\n\n"
        "If you want to continue your message in other cases, you can type '.' at the end of the line ('.' will be ignored in the message).\n"
        "Example:\n"
        "    $FOO .\n"
        "    .BAR\n"
        "Actual message:\n"
        "    FOO\n"
        "    BAR\n\n"
        "If you want to write freer, type '...' and then type enter. Then you can write as many lines as you want.\n"
        "Type '..' and then type enter to end this free writing.\n"
        "Example:\n"
        "    $...\n"
        "    FOO\n"
        "    BAR\n"
        "    ..\n"
        "Actual message:\n"
        "\n"
        "    FOO\n"
        "    BAR\n";
    static const char version[] =
        "are-client.exe v1.0";
    
    if(options & OPT_HELP) {
        puts(help);
        return true;
    }
    if(options & OPT_VERSION) {
        puts(version);
        return true;
    }
    return false;
}


Handle InputHandle(OPT options) {
    const char *res;
    if(options & OPT_QUIET) {
        res = WNAME_DEFAULT;
    } else {
        char name[WNAME_SIZE];
        printf_s("Window name (default: '%s'): ", WNAME_DEFAULT);
        gets_s(name, sizeof(name));
        res = name[0]? name : WNAME_DEFAULT;
    }
    return { FindWindowA(nullptr, res) };
}


void InputMsgQuiet(char message[], rsize_t msgsize);
void InputMsgConsole(char message[], rsize_t msgsize);

bool InputMsg(OPT options, char message[], rsize_t msgsize) {
    if(feof(stdin)) return false;
    if(options & OPT_QUIET)
        InputMsgQuiet(message, msgsize);
    else
        InputMsgConsole(message, msgsize);
    return true;
}


void InputMsgQuiet(char message[], rsize_t msgsize) {
    rsize_t len = fread(message, 1, msgsize-1, stdin);
    message[len] = '\0';
}


void InputMsgMultiLine(char message[], rsize_t msgsize);

void InputMsgConsole(char message[], rsize_t msgsize) {
    char *p = message; char c;
    int lbcnt = 0; int dotcnt = 0;
    putchar('$');
    while((c = getchar()) != EOF && p+1 < message+msgsize) {
        *(p++) = c;
        switch(c) {
        case '.':
            dotcnt++;
            break;
        case '\n':
            if(!lbcnt && !dotcnt) {
                *p = '\0';
                return;
            }
            if(dotcnt >= 3) {
                *((p-=3)-1) = '\n';
                InputMsgMultiLine(p, message+msgsize-p);
                return;
            } else if(dotcnt >= 1) {
                *((--p)-1) = '\n';
            }
            putchar('.');
        default:
            dotcnt = 0;
            break;
        }
        switch(c) {
            case '(': case '[': case '{':
                lbcnt++;
                break;
            case ')': case ']': case '}':
                lbcnt--;
                break;
        }
    }
    *p = '\0';
}


void InputMsgMultiLine(char message[], rsize_t msgsize) {
    char *p = message; char c;
    int dotcnt = 0;
    while((c = getchar()) != EOF && p+1 < message+msgsize) {
        *(p++) = c;
        switch(c) {
        case '.':
            dotcnt++;
            break;
        case '\n':
            if(dotcnt >= 2) {
                *((p-=2)-1) = '\0';
                return;
            }
        default:
            dotcnt = 0;
            break;
        }
    }
    *p = '\0';
}


void SendMsg(Handle handle, long long dwData, char message[], rsize_t msgsize) {
    COPYDATASTRUCT cData = { (ULONG_PTR)dwData, (DWORD)msgsize, message };
    SendMessageA(handle.hwnd, WM_COPYDATA, (WPARAM)nullptr, (LPARAM)&cData);
}


