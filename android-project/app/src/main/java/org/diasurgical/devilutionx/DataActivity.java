package org.diasurgical.devilutionx;

import android.app.Activity;
import android.app.DownloadManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.text.method.LinkMovementMethod;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;

public class DataActivity extends Activity {
	//private DownloadReceiver mReceiver;
	//private boolean isDownloading = false;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_data);

		((TextView) findViewById(R.id.full_guide)).setMovementMethod(LinkMovementMethod.getInstance());
		((TextView) findViewById(R.id.online_guide)).setMovementMethod(LinkMovementMethod.getInstance());
	}

	protected void onResume() {
		super.onResume();
		startGame();
	}

	public void startGame(View view) {
		startGame();
	}

	private void startGame() {
		Intent intent = new Intent(this, DevilutionXSDLActivity.class);
		startActivity(intent);
		this.finish();
	}

	protected void onDestroy() {
		//if (mReceiver != null)
		//	unregisterReceiver(mReceiver);

		super.onDestroy();
	}

	/**
	 * Start downloading the shareware
	 */
	/*public void sendDownloadRequest(View view) {
		String url = "https://github.com/d07RiV/diabloweb/raw/3a5a51e84d5dab3cfd4fef661c46977b091aaa9c/spawn.mpq";
		String fileName = "spawn.mpq";

		DownloadManager.Request request = new DownloadManager.Request(Uri.parse(url))
				.setTitle(fileName)
				.setDescription(getString(R.string.shareware_data))
				.setNotificationVisibility(DownloadManager.Request.VISIBILITY_VISIBLE);

		request.setDestinationInExternalFilesDir(this, null, fileName);

		if (mReceiver == null) {
			mReceiver = new DownloadReceiver();
			IntentFilter filter = new IntentFilter("android.intent.action.DOWNLOAD_COMPLETE");
			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
				registerReceiver(mReceiver, filter, Context.RECEIVER_EXPORTED);
			} else {
				registerReceiver(mReceiver, filter);
			}
		}

		DownloadManager downloadManager = (DownloadManager)this.getSystemService(Context.DOWNLOAD_SERVICE);
		downloadManager.enqueue(request);

		if (mReceiver == null)
			mReceiver = new DownloadReceiver();
		registerReceiver(mReceiver, new IntentFilter("android.intent.action.DOWNLOAD_COMPLETE"));

		isDownloading = true;
		view.setEnabled(false);

		Toast toast = Toast.makeText(getApplicationContext(), getString(R.string.download_started), Toast.LENGTH_SHORT);
		toast.show();
	}*/

	/**
	 * Start game when download finishes
	 */
	/*private class DownloadReceiver extends BroadcastReceiver {
		@Override
		public void onReceive(@NonNull Context context, @NonNull Intent intent) {
			isDownloading = false;

			long receivedID = intent.getLongExtra(DownloadManager.EXTRA_DOWNLOAD_ID, -1L);
			DownloadManager mgr = (DownloadManager) context.getSystemService(Context.DOWNLOAD_SERVICE);

			DownloadManager.Query query = new DownloadManager.Query();
			query.setFilterById(receivedID);
			Cursor cur = mgr.query(query);
			int index = cur.getColumnIndex(DownloadManager.COLUMN_STATUS);
			if (cur.moveToFirst()) {
				if (cur.getInt(index) == DownloadManager.STATUS_SUCCESSFUL) {
					startGame();
				}
			}
			cur.close();
			findViewById(R.id.download_button).setEnabled(true);
		}
	}*/
}
