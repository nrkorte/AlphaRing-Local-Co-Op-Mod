// Toggles the AlphaRing mod (WTSAPI32.dll) on/off for Halo MCC.
// OFF: renames the DLL to WTSAPI32.dll.disabled so the game ignores it (safe to launch with anti-cheat).
// ON:  renames it back. If no disabled copy exists, copies a WTSAPI32.dll sitting next to this exe.
// The MCC folder is found through Steam's library list.
using System;
using System.Diagnostics;
using System.IO;
using System.Text.RegularExpressions;
using System.Windows.Forms;
using Microsoft.Win32;

static class ToggleAlphaRing
{
    const string MccRelative = @"steamapps\common\Halo The Master Chief Collection\mcc\binaries\win64";
    const string Title       = "AlphaRing Toggle";

    [STAThread]
    static int Main()
    {
        string gameDir = FindGameDir();
        if (gameDir == null)
            return Fail("Could not find Halo MCC in any Steam library.");

        string active   = Path.Combine(gameDir, "WTSAPI32.dll");
        string disabled = Path.Combine(gameDir, "WTSAPI32.dll.disabled");
        string localDll = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "WTSAPI32.dll");

        if (Process.GetProcessesByName("MCC-Win64-Shipping").Length > 0)
            return Fail("Halo MCC is running. Close it first, then run this again.");

        try
        {
            if (File.Exists(active))
            {
                if (File.Exists(disabled)) File.Delete(disabled);
                File.Move(active, disabled);
                return Info("AlphaRing DISABLED\n\nLaunch MCC normally (anti-cheat OK).");
            }
            if (File.Exists(disabled))
            {
                File.Move(disabled, active);
                return Info("AlphaRing ENABLED\n\nLaunch MCC with anti-cheat OFF.");
            }
            if (File.Exists(localDll))
            {
                File.Copy(localDll, active);
                return Info("AlphaRing ENABLED (installed from " + localDll + ")\n\nLaunch MCC with anti-cheat OFF.");
            }
            return Fail("AlphaRing is not installed in:\n" + gameDir + "\n\nPut WTSAPI32.dll next to this exe and run it again to install it.");
        }
        catch (Exception e)
        {
            return Fail("Failed: " + e.Message);
        }
    }

    // Reads Steam's install path from the registry, then checks every library in libraryfolders.vdf.
    static string FindGameDir()
    {
        string steam = Registry.GetValue(@"HKEY_CURRENT_USER\Software\Valve\Steam", "SteamPath", null) as string;
        if (steam == null) return null;

        string vdf = Path.Combine(steam, @"steamapps\libraryfolders.vdf");
        string[] libraries = { steam };
        if (File.Exists(vdf))
        {
            MatchCollection matches = Regex.Matches(File.ReadAllText(vdf), "\"path\"\\s+\"([^\"]+)\"");
            libraries = new string[matches.Count + 1];
            libraries[0] = steam;
            for (int i = 0; i < matches.Count; i++)
                libraries[i + 1] = matches[i].Groups[1].Value.Replace(@"\\", @"\");
        }

        foreach (string library in libraries)
        {
            string dir = Path.Combine(library, MccRelative);
            if (Directory.Exists(dir)) return dir;
        }
        return null;
    }

    static int Info(string msg)
    {
        MessageBox.Show(msg, Title, MessageBoxButtons.OK, MessageBoxIcon.Information);
        return 0;
    }

    static int Fail(string msg)
    {
        MessageBox.Show(msg, Title, MessageBoxButtons.OK, MessageBoxIcon.Warning);
        return 1;
    }
}
