package org.diasurgical.devilutionx;

import android.content.Context;
import android.os.Build;

import java.io.File;

public class ExternalFilesManager {

	private static File chooseExternalFilesDirectory(Context context) {
		if (Build.VERSION.SDK_INT >= 19 /* Android 4.4 (KITKAT) */) {
			File[] externalDirs = context.getExternalFilesDirs(null);
			if (externalDirs != null) {
				for (File dir : externalDirs) {
					if (dir == null) {
						continue;
					}
					File[] iniFiles = dir.listFiles((dir1, name) -> name == "diablo.ini");
					if (iniFiles != null && iniFiles.length > 0) {
						return dir;
					}
				}

				for (File dir : externalDirs) {
					if (dir == null) {
						continue;
					}
					File[] anyFiles = dir.listFiles();
					if (anyFiles != null && anyFiles.length > 0) {
						return dir;
					}
				}
			}
		}

		// Fallback to the primary external storage directory
		return context.getExternalFilesDir(null);
	}

	public static String chooseExternalFilesDir(Context context) {
		File dir = chooseExternalFilesDirectory(context);

		return dir.getAbsolutePath() + "/";
	}

}
