package com.cse3345.f13.Tanner;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.nio.ByteBuffer;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.ColorMatrix;
import android.graphics.ColorMatrixColorFilter;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.hardware.Camera;
import android.hardware.Camera.PictureCallback;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends Activity {
	private Camera camera;
	private CameraPreview mPreview;
	private FrameLayout cameraPreview;
	private TextView selectedCameraStatus;
	private MenuItem menuFront;

	private boolean frontFlag, left;
	private int currCamera = -1, tempCamera;
	private String subjectID = "ID", mostRecentImageDir = null,
			mostRecentImageName, mostRecentTimeStamp;
	private byte[] toPass;

	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		selectedCameraStatus = (TextView) findViewById(R.id.Camera);
		left = true;
		cameraPreview = (FrameLayout) findViewById(R.id.surfaceView);

		makeCameraList();
		// set the camera preview to the default(back)
		setCamera(0);
		// set the selected Camera status text
		selectedCameraStatus.bringToFront();

		setSelectedCameraStatus();
	}

	@Override
	public void onDestroy() {
		super.onDestroy();
	}

	@Override
	public void onPause() {
		super.onPause();
		releaseCamera();

		tempCamera = currCamera;
		currCamera = -1;
	}

	@Override
	public void onResume() {
		super.onResume();
		setCamera(tempCamera);
		setSelectedCameraStatus();
	}

	@Override
	public void onBackPressed() {
		super.onBackPressed();

		// release the camera
		releaseCamera();

		onDestroy();
	}

	private void releaseCamera() {
		if (camera != null) {
			camera.release();
			camera = null;
		}
	}

	public void makeToast(Context context, String msg, int duration) {
		Toast.makeText(context, msg, duration).show();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu
		try {
			getMenuInflater().inflate(R.menu.main, menu);
		} catch (Exception e) {
			Log.e("TAG", "Error inflating menu");
			e.printStackTrace();
		}
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// menu selection
		switch (item.getItemId()) {
		case R.id.menu_back_camera:
			menuCameraSelection(0);
			return true;
		case R.id.menu_front_camera:
			menuCameraSelection(1);
			return true;
		default:
			return super.onOptionsItemSelected(item);
		}
	}

	// make list of cameras
	public void makeCameraList() {
		int numOfCams = Camera.getNumberOfCameras();

		if (numOfCams == 1)
			frontFlag = false;
		else
			frontFlag = true;
	}

	@Override
	public boolean onPrepareOptionsMenu(Menu menu) {
		// add the front camera as a menu option
		menuFront = (MenuItem) menu.findItem(R.id.menu_front_camera);

		if (!frontFlag)
			menuFront.setVisible(false);
		else
			menuFront.setVisible(true);

		return super.onPrepareOptionsMenu(menu);
	}

	public void setSelectedCameraStatus() {
		if (currCamera == 0)
			selectedCameraStatus.setText(R.string.back_camera);
		else if (currCamera == 1)
			selectedCameraStatus.setText(R.string.front_camera);
	}

	public void setCamera(int i) {
		if (i == currCamera)
			return;

		// release camera
		releaseCamera();

		// set which camera is displayed
		currCamera = i;
		// clear all views from the Frame Layout
		if (cameraPreview.getChildCount() > 0)
			cameraPreview.removeAllViews();

		camera = getCamera(i);

		Camera.Parameters params = camera.getParameters();

		if (camera.getParameters().isZoomSupported())
			params.setZoom(camera.getParameters().getMaxZoom() / 4);

		camera.setParameters(params);

		// set preview
		mPreview = new CameraPreview(this, camera);

		// add the view
		cameraPreview.addView(mPreview);
		camera.startPreview();

	}

	@Override
	protected void onActivityResult(int req, int result, Intent data) {
		super.onActivityResult(req, result, data);
		left = data.getBooleanExtra("leftEye", true);
		mostRecentImageName = data.getStringExtra("fileName");
		subjectID = data.getStringExtra("subjectID");
		// update the textView for subjectID
		if (data.getBooleanExtra("save", false))
			saveToPhone();
		return;
	}

	private void saveToPhone() {
		sendBroadcast(new Intent(Intent.ACTION_MEDIA_MOUNTED,
				Uri.parse("file://" + mostRecentImageDir)));
	}

	public void menuCameraSelection(int i) {
		// set camera
		setCamera(i);
		// show which camera is selected
		setSelectedCameraStatus();
	}

	// take the picture
	public void takePicture(View view) {
		camera.takePicture(null, null, mPicture);

		camera.startPreview();

	}

	// open camera
	public Camera getCamera(int i) {
		Camera cam = null;
		try {
			cam = Camera.open(i);
			cam.setDisplayOrientation(90);
		} catch (Exception e) {
		}
		return cam;
	}

	// start the next activity
	public void beginSegmentActivity() {
		Intent intent = new Intent(getBaseContext(), SegmentActivity.class);
		// put the variables needed in the next activity in an intent
		intent.putExtra("subjectID", subjectID);
		intent.putExtra("fileDir", mostRecentImageDir);
		intent.putExtra("fileName", mostRecentImageName);
		intent.putExtra("leftEye", left);
		intent.putExtra("timeStamp", mostRecentTimeStamp);
		intent.putExtra("toPass", toPass);
		intent.putExtra("save", false);
		startActivityForResult(intent, 0);

	}

	public File getOutputMediaFile() {
		// make sure subjectID is up to date with the tempString
		File mediaStorageDir = new File(
				Environment
						.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES),
				"Captured_Images");
		if (!mediaStorageDir.exists()) {
			if (!mediaStorageDir.mkdirs()) {
				Log.d("TAG", "failed to create directory");
				return null;
			}
		}
		// Create a media file name
		String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss", Locale.US)
				.format(new Date());

		File mediaFile;

		mediaFile = new File(mediaStorageDir.toString(), subjectID + "_"
				+ timeStamp + ".jpg");

		// store the path as a string
		mostRecentImageDir = mediaStorageDir.toString() + "/";
		mostRecentImageName = subjectID + "_" + timeStamp + ".jpg";
		mostRecentTimeStamp = timeStamp;

		return mediaFile;
	}

	// This function was not created by me. I found it on stackoverflow here:
	// http://stackoverflow.com/questions/3373860/convert-a-bitmap-to-grayscale-in-android/3391061#3391061
	// I needed a way to make an iamge taken on the camera grayscale
	// and I just don't have the image processing knowledge to know how to do
	// that properly (I tried my own method
	// and the results looked very bad)
	// I did change some of the variable names because I didn't like how short
	// and undescriptive some of them were and it just looked bad to me
	public Bitmap toGrayscale(Bitmap original) {
		int width, height;
		height = original.getHeight();
		width = original.getWidth();

		Bitmap grayscale = Bitmap.createBitmap(width, height,
				Bitmap.Config.RGB_565);
		Canvas canvas = new Canvas(grayscale);
		Paint paint = new Paint();
		ColorMatrix matrix = new ColorMatrix();
		matrix.setSaturation(0);
		ColorMatrixColorFilter filter = new ColorMatrixColorFilter(matrix);
		paint.setColorFilter(filter);
		canvas.drawBitmap(original, 0, 0, paint);
		return grayscale;
	}

	PictureCallback mPicture = new PictureCallback() {
		@Override
		public void onPictureTaken(byte[] data, Camera camera) {

			File pictureFile = getOutputMediaFile();
			if (pictureFile == null) {
				return;
			}

			Bitmap image = BitmapFactory.decodeByteArray(data, 0, data.length);
			Matrix matrix = new Matrix();
			// m.postRotate(90); // used to rotate the image but is no longer
			// needed
			image = Bitmap.createBitmap(image, 0, 0, image.getWidth(),
					image.getHeight(), matrix, true);

			ByteArrayOutputStream outStream = new ByteArrayOutputStream();
			image = toGrayscale(image);
			image.compress(Bitmap.CompressFormat.PNG, 100, outStream);

			// This code here is for taking the PNG image that the camera
			// provides and converting it to an array of bytes that then
			// can be directly used as BMP using the c++ code implemented with
			// JNI
			// this is because the segmentation has to use a 256 color
			// (grayscale) BMP because it was originally designed
			// to use on images taken at near infrared wavelengths which appear
			// as grayscale images. Since the images were already grayscale
			// it made sense to store them in a small format that was already
			// grayscale itself (256 color BMPs)
			int ARGB;
			byte[] alphaGet = new byte[480 * 640];
			byte[] tempARGB = new byte[4];
			byte Red;
			byte Green;
			byte Blue;
			int counter = 0;

			for (int i = 0; i < 480; i++) {
				for (int j = 0; j < 640; j++) {
					ARGB = image.getPixel(j, i);
					tempARGB = ByteBuffer.allocate(4).putInt(ARGB).array();
					Red = tempARGB[1];
					Green = tempARGB[2];
					Blue = tempARGB[3];

					alphaGet[counter] = (byte) ((Red + Green + Blue) / 3);
					counter++;
				}
			}
			toPass = alphaGet;
			// start next activity
			beginSegmentActivity();
		}
	};
}