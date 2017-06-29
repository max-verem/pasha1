#include <windows.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <errno.h>

// https://stackoverflow.com/questions/865152/how-can-i-get-a-process-handle-by-its-name-in-c
static int get_pid(const char* name)
{
    int pid = -ENOENT;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 process;
    ZeroMemory(&process, sizeof(process));
    process.dwSize = sizeof(process);
    if(Process32First(snapshot, &process))
    {
        do
        {
            if (!stricmp(process.szExeFile, name))
            {
                pid = process.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &process));
    };

    CloseHandle(snapshot);

    return pid;
};

typedef struct proc_coord_desc
{
    int pid, x, y, w, h;
} proc_coord_t;

BOOL CALLBACK _get_process_coords(HWND hwnd, LPARAM lParam)
{
    DWORD pid, th;
    proc_coord_t* pc = (proc_coord_t*)lParam;

    th = GetWindowThreadProcessId(hwnd, &pid);

    if (pc->pid == pid)
    {
        RECT r;

        ::GetWindowRect(hwnd, &r);

        pc->x = r.left;
        pc->y = r.top;
        pc->w = r.right - r.left;
        pc->h = r.bottom - r.top;

        return FALSE;
    };

    return TRUE;
}

static int get_process_coords(proc_coord_t* pc)
{
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pc->pid);
    if(hProcess)
    {
        EnumWindows(_get_process_coords, (LPARAM)pc);
        CloseHandle(hProcess);
    };

    return (!pc->w || !pc->h) ? -ENOENT : 0;
}

const char* PROC_NAME = "notepad.exe";

int main(int argc, char** argv)
{
    proc_coord_t pc_prev;

    ZeroMemory(&pc_prev, sizeof(pc_prev));

    while (1)
    {
        int r;
        proc_coord_t pc_curr;

        r = get_pid(PROC_NAME);
        if(r < 0)
        {
            printf("no [%s] found\n", PROC_NAME);
            Sleep(1000);
            continue;
        }

        ZeroMemory(&pc_curr, sizeof(pc_curr));
        pc_curr.pid = r;
        r = get_process_coords(&pc_curr);
        if(!r && (pc_curr.x != pc_prev.x || pc_curr.y != pc_prev.y || pc_curr.w != pc_prev.w || pc_curr.h != pc_prev.h))
            fprintf(stderr, "%s:%d: x=%5d, y=%5d, w=%5d, h=%5d\n", __FUNCTION__, __LINE__,
                pc_curr.x, pc_curr.y, pc_curr.w, pc_curr.h);
        pc_prev = pc_curr;

        Sleep(40);
    }
}