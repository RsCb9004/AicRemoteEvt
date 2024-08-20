using System;
using System.Runtime.InteropServices;

namespace AicRemoteEvt {

    /// <summary>
    /// A class that can send message to other processes based on Windows <c>user32.dll</c>.
    /// </summary>
    public class WinHookMsgGetter : IDisposable {

        private struct COPYDATASTRUCT {
            public ulong dwData;
            public int cData;
            [MarshalAs(UnmanagedType.LPStr)]
            public string lpData;
        }

        private struct CWPSTRUCT {
            public IntPtr lParam;
            public IntPtr wParam;
            public uint message;
            public IntPtr hwnd;
        }

        private const int WH_CALLWNDPROC = 4;
        private const int WM_COPYDATA = 0x004A;

        private const ulong DWDATA_DEFAULT = 0x6d756b6f75616f69;

        private delegate int HookProc(int nCode, IntPtr wParam, IntPtr lParam);

        [DllImport("user32.dll")]
        private static extern IntPtr SetWindowsHookEx(int idHook, HookProc lpfn, IntPtr hInstance, uint dwThreadId);
        [DllImport("user32.dll")]
        private static extern int CallNextHookEx(IntPtr hhk, int nCode, IntPtr wParam, IntPtr lParam);
        [DllImport("user32.dll")]
        private static extern bool UnhookWindowsHookEx(IntPtr hhk);

        [DllImport("kernel32.dll")]
        public static extern uint GetCurrentThreadId();


        private IntPtr hook;
        // IDK why but this variable is needed.
        // But if not, "this" captured into the delegate will be null after showing starting screen of Unity.
        private readonly HookProc proc;

        /// <summary>
        /// The accepted value of <c><see cref="COPYDATASTRUCT.dwData"></see></c>.
        /// <c><see cref="COPYDATASTRUCT.dwData"></see></c> which is not this value will be ignored.
        /// </summary>
        public ulong AcceptedDwData { get; }
        /// <summary>
        /// The callback after recieving message.
        /// </summary>
        public Action<string> OnGetMsg { get; }

        /// <summary>
        /// Set a hook listening to any messages sent to this process.
        /// </summary>
        /// <param name="onGetMsg"> The callback after recieving message. </param>
        public WinHookMsgGetter(Action<string> onGetMsg = null, ulong acceptedDwData = DWDATA_DEFAULT) {
            AcceptedDwData = acceptedDwData;
            OnGetMsg = onGetMsg;
            hook = IntPtr.Zero;
            proc = CallWndProc;
            Hook();
        }


        private unsafe int CallWndProc(int nCode, IntPtr wParam, IntPtr lParam) {
            var cwpMsg = Marshal.PtrToStructure<CWPSTRUCT>(lParam);
            if(cwpMsg.message == WM_COPYDATA) {
                var cData = Marshal.PtrToStructure<COPYDATASTRUCT>(cwpMsg.lParam);
                if(cData.dwData == AcceptedDwData)
                    OnGetMsg?.Invoke(cData.lpData);
            }
            return CallNextHookEx(hook, nCode, wParam, lParam);
        }

        /// <summary>
        /// Set the hook, start recieving message.
        /// </summary>
        public void Hook() {
            if(hook != IntPtr.Zero) return;
            uint threadId = GetCurrentThreadId();
            hook = SetWindowsHookEx(WH_CALLWNDPROC, proc, IntPtr.Zero, threadId);
        }

        /// <summary>
        /// Unset the hook, stop recieving message.
        /// </summary>
        public void Unhook() {
            if(hook == IntPtr.Zero) return;
            UnhookWindowsHookEx(hook);
        }

        public void Dispose() {
            Unhook();
        }

        ~WinHookMsgGetter() {
            Dispose();
        }
    }
}
