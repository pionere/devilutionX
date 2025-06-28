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

public class PatcherActivity extends SDLActivity {
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
	}

/*private Activity.finishAndRemoveTaskCompat() {
    if (android.os.Build.VERSION.SDK_INT >= 21) {
        finishAndRemoveTask()
    } else {
        Intent intent = Intent(this, ExitAndRemoveFromRecentAppsDummyActivity.class)

        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK |
                Intent.FLAG_ACTIVITY_CLEAR_TASK |
                Intent.FLAG_ACTIVITY_NO_ANIMATION |
                Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);

        startActivity(intent);
    }
}

class ExitAndRemoveFromRecentAppsDummyActivity extends Activity {
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        finish();
    }
}*/

	/**
	 * When the user exits the game, use System.exit(0)
	 * to clear memory and prevent errors on restart
	 */
	protected void onDestroy() {
		super.onDestroy();

		//super.finish();
		// if (Build.VERSION.SDK_INT >= 21 /* Build.VERSION_CODES.LOLLIPOP*/) {
		//	finishAndRemoveTask();
		// }
		// System.exit(0);
		// android.os.Process.killProcess(android.os.Process.myPid());
	}

	private File chooseExternalFilesDirectory() {
		if (Build.VERSION.SDK_INT >= 19 /* Android 4.4 (KITKAT) */) {
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
				"devil_patcher"
		};
	}
}
