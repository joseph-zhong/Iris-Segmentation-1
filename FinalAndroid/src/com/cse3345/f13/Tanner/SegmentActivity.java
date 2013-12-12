package com.cse3345.f13.Tanner;

import java.io.File;


import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

public class SegmentActivity extends Activity {
	private String currId, currFileName, sourceDir,
			sourceFileName, timestamp, eye, newDir;

	private String sourceSegDir, sourceSegFilename,
			newSegDir, newSegFilename;
	
	private TextView idTextView, previewLabelTextView;
	private EditText idEditText;
	private RadioGroup eyeSel;
	private RadioButton radioLeft;
	private Button saveButton;
	private ImageView image;
	private Intent returnIntent, sourceIntent;
	private boolean left, discard = false;
	private Bitmap capturedImage, segmentedImage;
	public byte[] imgBytes;
	boolean save = false;

	public void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);
		setContentView(R.layout.segment_activity);

		returnIntent = new Intent();

		idTextView = (TextView) findViewById(R.id.Subject_ID);
		previewLabelTextView = (TextView) findViewById(R.id.Preview_Label);
		image = (ImageView) findViewById(R.id.Preview);
		idEditText = (EditText) findViewById(R.id.subject_ID_editText);
		eyeSel = (RadioGroup) findViewById(R.id.EyeSelection);
		radioLeft = (RadioButton) findViewById(R.id.radioLeft);
		saveButton = (Button) findViewById(R.id.Save_Button);
		sourceIntent = getIntent();

		// set the strings for the image
		currId = sourceIntent.getStringExtra("subjectID");
		currFileName = sourceIntent.getStringExtra("fileName");
		sourceFileName = currFileName;
		sourceDir = sourceIntent.getStringExtra("fileDir");
		timestamp = sourceIntent.getStringExtra("timeStamp");
		eye = "_L_";
		imgBytes = sourceIntent.getByteArrayExtra("toPass");

		// set other vairiables from the passed intent
		left = sourceIntent.getBooleanExtra("leftEye", true);

		// initilize the values in the textViews to the passed values
		idTextView.setText("Subject ID: ");
		idEditText.setText(currId);
		previewLabelTextView.setText("Captured Image:\n" + currFileName);

		// disable the save button until there is a result
		saveButton.setEnabled(false);

		// start segmentation
		new segment().execute(sourceDir, sourceFileName);

		// display the captured image
		capturedImage = BitmapFactory.decodeFile(sourceDir + currFileName);
		image.setImageBitmap(capturedImage);

		
		
		
		idEditText.addTextChangedListener(new TextWatcher() {

			@Override
			public void afterTextChanged(Editable s) {
				// set the value of subject ID
				currId = s.toString();

				// update the string displayed on the screen
				updateFilename();
			}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count,
					int after) {
				//unused
				
			}

			@Override
			public void onTextChanged(CharSequence s, int start, int before,
					int count) {
				// unused in this method
				
			}
		});
		

		// set which eye it is based on radio buttons
		if (left != true) {
			eyeSel.check(R.id.radioRight);
			eye = "_R_";
		}

		// update the filename that is displayed on the screen in the textView
		updateFilename();

	}

	@Override
	public void onBackPressed() {
		confirmDiscard(getCurrentFocus());
	}

	public void getOrigDir() {
		// make sure subjectID is up to date with the tempString
		File dir = new File(
				Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES)
						+ "/Captured_Images", currId);
		if (!dir.exists()) {
			if (!dir.mkdirs()) {
				Log.d("TAG", "failed to create directory");
				makeToast(this,
						"Error! Directory could not be found or created.",
						Toast.LENGTH_LONG);
				return;
			}
		}

		// save the new directory
		newDir = dir.toString() + "/";
	}

	// sets the new storage directory for the segmented images
	public void getNewSegDir() {
		// make sure subjectID is up to date with the tempString
		File dir = new File(
				Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES)
						+ "/Captured_Images/" + currId, "Segmented");
		if (!dir.exists()) {
			if (!dir.mkdirs()) {
				Log.d("TAG", "failed to create directory");
				makeToast(this,
						"Error! Directory could not be found or created.",
						Toast.LENGTH_LONG);
				return;
			}
		}

		// save the new directory
		newSegDir = dir.toString() + "/";
	}

	public void updateFilename() {
		// set the filename for the original image
		currFileName = currId + eye + timestamp;

		// set the filename for the segmented image
		newSegFilename = "SEG_" + currId + eye
				+ timestamp;

		// update the textView
		previewLabelTextView.setText("Captured Image:\n" + currFileName);

		return;
	}

	// onclick for radio group to select eye
	public void radioGroupEyeSelection(View view) {
		int choice = eyeSel.getCheckedRadioButtonId();

		if (choice == radioLeft.getId()) {
			eye = "_L_";
			left = true;
		} else {
			eye = "_R_";
			left = false;
		}

		// update the filename
		updateFilename();
	}

	public void makeToast(Context context, String msg, int duration) {
		Toast.makeText(context, msg, duration).show();
	}

	// onClick method for the save button
	public void saveButton(View view) {
		// make sure name is updated
		updateFilename();
		// update the directory
		getOrigDir();

		// rename the original image
		try {
			File old = new File(sourceDir + sourceSegFilename);
			File newFile = new File(newDir + newSegFilename + ".jpg");
			old.renameTo(newFile);
		} catch (Exception e) {
			Log.e("TAG", "Error renaming the file");
			e.printStackTrace();
			makeToast(this, "There was an error renaming the file",
					Toast.LENGTH_LONG);
		}

			// update the directory path
			//getNewSegDir();

			//rename segmented image
			try {
				File oldSegFile = new File(sourceSegDir
						+ sourceSegFilename);
				File newSegFile = new File(newSegDir
						+ newSegFilename);
				oldSegFile.renameTo(newSegFile);
			} catch (Exception e) {
				Log.e("TAG", "Error renaming the file");
				e.printStackTrace();
				makeToast(this, "There was an error renaming the file",
						Toast.LENGTH_LONG);
			}
		

		//add stuff to intent
		returnIntent.putExtra("subjectID", currId);
		returnIntent.putExtra("fileName", newSegFilename);
		returnIntent.putExtra("leftEye", left);
		returnIntent.putExtra("save", true);

		setResult(0, returnIntent);

		// toast that the image has been saved
		makeToast(this, "The image has been saved!", Toast.LENGTH_LONG);

		finish();
	}

	// onClick for the discard button
	public void discardButton(View view) {
		discard = true;
		
		try {
			File file = new File(sourceDir + sourceFileName);
			file.delete();
		} catch (Exception e) {
			Log.e("TAG", "There was an error deleting the file");
			e.printStackTrace();
			makeToast(this, "There was an error deleting the file",
					Toast.LENGTH_LONG);
		}

		// delete the segmented image
		try {
			File segFile = new File(sourceSegDir
					+ sourceSegFilename);
			segFile.delete();
		} catch (Exception e) {
			Log.e("TAG", "There was an error deleting the file");
			e.printStackTrace();
			makeToast(this, "There was an error deleting the file",
					Toast.LENGTH_LONG);
		}

		
		returnIntent.putExtra("subjectID", currId);
		returnIntent.putExtra("fileName", (String) null);
		returnIntent.putExtra("leftEye", left);
		returnIntent.putExtra("save", false);

		setResult(0, returnIntent);

		finish();
	}

	// confimration pop-up for discarding an image
	// prompts the user for a string and sets subjectID
	public void confirmDiscard(final View view) {
		AlertDialog.Builder alert = new AlertDialog.Builder(this);

		alert.setTitle("Delete File");
		alert.setMessage("This can not be undone.");

		alert.setPositiveButton("Delete", new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int whichButton) {
				discardButton(view);
			}
		});

		alert.setNegativeButton("Cancel",
				new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int whichButton) {
						return;
					}
				});

		alert.show();
	}

	// displays the segmented image, and enables the save button
	public void segComplete() {
		if (!discard) {
			saveButton.setEnabled(true);

			// changes the image displayed from the captured image to the segmented
			storeImage();

			segmentedImage = BitmapFactory.decodeFile(sourceDir
					+ sourceSegFilename);
			image.setImageBitmap(segmentedImage);

			makeToast(getBaseContext(), "Segmenting Complete!",
					Toast.LENGTH_LONG);
		} else {
			try {
				File segFile = new File(sourceSegDir
						+ sourceSegFilename);
				segFile.delete();
			} catch (Exception e) {
				Log.e("TAG", "There was an error deleting the file");
				e.printStackTrace();
				makeToast(this, "There was an error deleting the file",
						Toast.LENGTH_LONG);
			}
		}

	}

	private void storeImage() {
		sendBroadcast(new Intent(Intent.ACTION_MEDIA_MOUNTED,
				Uri.parse("file://" + sourceDir)));
	}

	private class segment extends AsyncTask<String, Void, Boolean> {

		public String charToString(char[] array, int start, int last) {
			return new String(array, start, last - start);
		}

		@Override
		protected void onPreExecute() {
			makeToast(getBaseContext(), "Segmenting...", Toast.LENGTH_LONG);
		}

		protected Boolean doInBackground(String... args) {

			args[0] += args[1];
			String pathName = args[0];

			pathName = pathName.substring(0, pathName.lastIndexOf("."));

			Eye stats = new Eye();

			String fileName = new String();
			String path = new String();
			String newFileName = new String();
			char[] charStr = pathName.toCharArray();

			int last = 0;

			for (int i = 0; i < charStr.length; i++) {
				if (charStr[charStr.length - i - 1] == '/') {

					last = charStr.length - i - 1;

					path = charToString(charStr, 0, last + 1);
					fileName = charToString(charStr, last + 1, last + i + 1);
					break;
				}
			}
			newFileName = fileName + "_Segmented";
			sourceSegFilename = newFileName + ".bmp";
			newSegFilename = newFileName + ".bmp";
			Log.w("newSegFilename", newSegFilename);
			Log.w("path", path);
			Log.w("fileName", fileName);
			Log.w("newFileName", newFileName);

			JNI test = new JNI();

			Log.w("test", "Pre segment");
			test.Segment(fileName, path, stats, imgBytes);
			Log.w("test", "post segment");

			return true;
		}

		protected void onPostExecute(Boolean result) {
			segComplete();
		}

	}
}
