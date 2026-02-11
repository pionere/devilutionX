package org.diasurgical.devilutionx;

import android.graphics.Rect;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.ViewTreeObserver;
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
		super.onCreate(savedInstanceState);

		// windowSoftInputMode=adjustPan stopped working
		// for fullscreen apps after Android 7.0
		if (Build.VERSION.SDK_INT >= 25 /* Android 7.1 (N_MR1) */ )
			trackVisibleSpace();

		externalDir = ExternalFilesManager.chooseExternalFilesDir(this);
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
				if (surface != null) {
					SurfaceHolder holder = surface.getHolder();
					holder.setFixedSize(visibleSpace.width(), visibleSpace.height());
				}
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
