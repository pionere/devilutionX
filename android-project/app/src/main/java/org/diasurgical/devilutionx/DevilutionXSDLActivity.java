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
import android.view.WindowManager;

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

		// Force app to overlap with the display cutout
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P)
			getWindow().getAttributes().layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_ALWAYS;

		externalDir = chooseExternalFilesDir();

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

	private File chooseExternalFilesDirectory() {
		if (Build.VERSION.SDK_INT >= 19) {
			File[] externalDirs = getExternalFilesDirs(null);
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
		return getExternalFilesDir(null);
	}

	private String chooseExternalFilesDir() {
		File dir = chooseExternalFilesDirectory();

		return dir.getAbsolutePath() + "/";
	}

	/**
	 * Check if the game data is present
	 */
	private boolean missingGameData() {
		File fileDev = new File(externalDir + "devilx.mpq");
		if (!fileDev.exists())
			return true;

		File fileLower = new File(externalDir + "diabdat.mpq");
		File fileUpper = new File(externalDir + "DIABDAT.MPQ");
		//File spawnFile = new File(externalDir + "spawn.mpq");

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

	protected String[] getArguments() {
		/*if (BuildConfig.DEBUG) {
			return new String[]{
				"--data-dir",
				externalDir,
				"--save-dir",
				externalDir,
				"--verbose",
			};
		}*/

		return new String[]{
			"--data-dir",
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
