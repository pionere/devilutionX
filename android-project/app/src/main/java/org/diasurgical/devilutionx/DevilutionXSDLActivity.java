package org.diasurgical.devilutionx;

import android.content.Intent;
import android.graphics.Rect;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.ViewTreeObserver;
import android.widget.Toast;

import org.libsdl.app.SDLActivity;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Locale;

public class DevilutionXSDLActivity extends SDLActivity {
	private String externalDir;

	protected void onCreate(Bundle savedInstanceState) {
		// windowSoftInputMode=adjustPan stopped working
		// for fullscreen apps after Android 7.0
		if (Build.VERSION.SDK_INT >= 25)
			trackVisibleSpace();

		externalDir = chooseExternalFilesDir();

		migrateSaveGames();

		super.onCreate(savedInstanceState);
	}

	/**
	 * On app launch make sure the game data is present
	 */
	protected void onStart() {
		super.onStart();

		if (missingGameData()) {
			Toast toast = Toast.makeText(DevilutionXSDLActivity.this, getString(R.string.missing_game_data), Toast.LENGTH_SHORT);
			toast.show();

			if (Build.VERSION.SDK_INT >= 30) {
				toast.addCallback(new android.widget.Toast.Callback(){
					public void onToastShown() {
						super.onToastShown();
					}

					public void onToastHidden() {
						super.onToastHidden();
						DevilutionXSDLActivity.this.finish();
					}
				  });
			} else {
				new Handler().postDelayed(new Runnable() {
					@Override
					public void run() {
						DevilutionXSDLActivity.this.finish();
					}
				}, Toast.LENGTH_SHORT * 1000);
			}
			return;
		}
	}

	/**
	 * When the user exits the game, use System.exit(0)
	 * to clear memory and prevent errors on restart
	 */
	protected void onDestroy() {
		super.onDestroy();

		System.exit(0);
	}

	private String chooseExternalFilesDir() {
		if (Build.VERSION.SDK_INT >= 19) {
			File[] externalDirs = getExternalFilesDirs(null);

			for (int i = 0; i < externalDirs.length; i++) {
				File dir = externalDirs[i];
				File[] iniFiles = dir.listFiles((dir1, name) -> name == "diablo.ini");
				if (iniFiles.length > 0)
					return dir.getAbsolutePath();
			}

			for (int i = 0; i < externalDirs.length; i++) {
				File dir = externalDirs[i];
				if (dir.listFiles().length > 0)
					return dir.getAbsolutePath();
			}
		}

		return getExternalFilesDir(null).getAbsolutePath();
	}

	/**
	 * Check if the game data is present
	 */
	private boolean missingGameData() {
		File fileDev = new File(externalDir + "/devilx.mpq");
		if (!fileDev.exists())
			return true;

		File fileLower = new File(externalDir + "/diabdat.mpq");
		File fileUpper = new File(externalDir + "/DIABDAT.MPQ");
		//File spawnFile = new File(externalDir + "/spawn.mpq");

		return !fileUpper.exists() && !fileLower.exists(); // && (!spawnFile.exists() || isDownloading);
	}

	private void trackVisibleSpace() {
		this.getWindow().getDecorView().getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
			@Override
			public void onGlobalLayout() {
				// Software keyboard may encroach on the app's visible space so
				// force the drawing surface to fit in the visible display frame
				Rect visibleSpace = new Rect();
				getWindow().getDecorView().getWindowVisibleDisplayFrame(visibleSpace);

				SurfaceView surface = mSurface;
				SurfaceHolder holder = surface.getHolder();
				holder.setFixedSize(visibleSpace.width(), visibleSpace.height());
			}
		});
	}

	private boolean copyFile(File src, File dst) {
		try {
			InputStream in = new FileInputStream(src);
			try {
				OutputStream out = new FileOutputStream(dst);
				try {
					// Transfer bytes from in to out
					byte[] buf = new byte[1024];
					int len;
					while ((len = in.read(buf)) > 0) {
						out.write(buf, 0, len);
					}
				} finally {
					out.close();
				}
			} finally {
				in.close();
			}
		} catch (IOException exception) {
			String message = exception.getMessage();
			if (message == null) {
				Log.e("copyFile", "IOException", exception);
			} else {
				Log.e("copyFile", message);
			}
			if (dst.exists()) {
				//noinspection ResultOfMethodCallIgnored
				dst.delete();
			}
			return false;
		}

		return  true;
	}

	private void migrateFile(File file) {
		//if (!file.exists() || !file.canRead()) {
		//	return;
		//}
		File newPath = new File(externalDir + "/" + file.getName());
		if (newPath.exists()) {
			if (file.canWrite()) {
				//noinspection ResultOfMethodCallIgnored
				file.delete();
			}
			return;
		}
		//if (!new File(newPath.getParent()).canWrite()) {
		//	return;
		//}
		if (!file.renameTo(newPath)) {
			if (copyFile(file, newPath) && file.canWrite()) {
				//noinspection ResultOfMethodCallIgnored
				file.delete();
			}
		}
	}

	private void migrateSaveGames() {
		File[] files = getFilesDir().listFiles();
		if (files == null)
			return;
		for (File internalFile : files) {
			migrateFile(internalFile);
		}
	}

	/**
	 * This method is called by SDL using JNI.
	 */
	public String getLocale()
	{
		return Locale.getDefault().toString();
	}

	protected String[] getArguments() {
		if (BuildConfig.DEBUG) {
			return new String[]{
				"--data-dir",
				externalDir,
				"--config-dir",
				externalDir,
				"--save-dir",
				externalDir,
				"--verbose",
			};
		}

		return new String[]{
			"--data-dir",
			externalDir,
			"--config-dir",
			externalDir,
			"--save-dir",
			externalDir
		};
	}

	protected String[] getLibraries() {
		return new String[]{
				//"SDL2",
				//"SDL2_mixer",
				//"SDL2_ttf",
				"devilutionx"
		};
	}
}
