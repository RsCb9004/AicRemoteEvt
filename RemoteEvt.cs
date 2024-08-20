using BepInEx;
using evt;
using HarmonyLib;
using UnityEngine;

namespace AicRemoteEvt {

    [BepInPlugin("rscb.aic-remote-evt", "AicRemoteEvt", "1.0.0")]
    public class RemoteEvt : BaseUnityPlugin {

        public static RemoteEvt Instance { get; private set; }


        [HarmonyPrefix]
        [HarmonyPatch(typeof(EV), "getEventContent")]
        public static bool GetEvtContentPrefix(string _name, EvReader ER, ref bool __result) {
            if(!_name.StartsWith("$")) return true;
            string content = _name.Substring(1);
            Instance.Logger.LogInfo($"Command: {content}");
            ER?.parseText(content);
            __result = true;
            return false;
        }


        public static void RunCommand(string command) {
            EV.stack("$" + command);
        }

        private WinHookMsgGetter hook;

        private string curCommand;

        public void Awake() {
            if(Application.platform != RuntimePlatform.WindowsPlayer)
                Logger.LogError("This plugin can only run in Windows.");
            Instance = this;
            curCommand = null;
            Harmony.CreateAndPatchAll(GetType());
            hook = new WinHookMsgGetter(OnGetMessage);
            Logger.LogInfo("Hello, world!");
        }

        public void CanCanNeed() {
            Debug.Log(hook);
        }

        public void Update() {
            if(curCommand != null) {
                RunCommand(curCommand);
                curCommand = null;
            }
        }

        private void OnGetMessage(string message) {
            curCommand = message;
        }

        /*
		private const string ResNamespaceName = "AicRemoteEvt";

		private static void CreateFileFromRes(string fileName) {
			string resName = $"{ResNamespaceName}.{fileName}";
			using(var resStream = typeof(RemoteEvt).Assembly.GetManifestResourceStream(resName))
			using(var fileStream = File.Create(fileName)) {
				resStream.CopyTo(fileStream);
			}
			Instance.Logger.LogInfo($"Created {fileName}.");
		}
		*/


        // There's a bug in fucking Mono. So we can't use this fucking shit.
        /*
		private const string PipeExeName = "pipe-transfer.exe";
		private static Stream StartPipeProcess() {
			var process = new Process();
			process.StartInfo.FileName = PipeExeName;
			process.StartInfo.UseShellExecute = false;
			process.StartInfo.RedirectStandardOutput = true;
			process.Start();
			return process.StandardOutput.BaseStream;
		}
		*/

        /*
		private static async Task<byte[]> ReadBytesAsync(Stream stream, int length) {
			byte[] res = new byte[length];
			await stream.ReadAsync(res, 0, length);
			return res;
		}

		private static async Task<string> ReadStringAsync(Stream stream) {
			int len = BitConverter.ToUInt16(await ReadBytesAsync(stream, 2), 0);
			return Encoding.UTF8.GetString(await ReadBytesAsync(stream, len));
		}
		*/
    }
}
