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
	private String currId, currFileName, sourceDir, sourceFileName, timestamp,
			eye, newDir, sourceSegDir, sourceSegFilename, newSegDir,
			newSegFilename;

	private TextView idTextView, previewLabelTextView;
	private EditText idEditText;
	private RadioGroup eyeSel;
	private RadioButton radioLeft;
	private Button saveButton;
	private ImageView image;
	private Intent returnIntent, sourceIntent;
	private boolean left, delete = false;
	private Bitmap capturedImage, segmentedImage;
	public byte[] imgBytes;

	public void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);
		setContentView(R.layout.segment_activity);

		returnIntent = new Intent();

		idTextView = (TextView) findViewById(R.id.Subject_ID);
		previewLabelTextView = (TextView) findViewById(R.id.Preview_Label);
		image = (ImageView) findViewById(R.id.Preview);
		idEditText = (EditText) findViewById(R.id.ID_editText);
		eyeSel = (RadioGroup) findViewById(R.id.EyeSelection);
		radioLeft = (RadioButton) findViewById(R.id.radioLeft);
		saveButton = (Button) findViewById(R.id.Save_Button);
		sourceIntent = getIntent();

		// set variables from intent
		currId = sourceIntent.getStringExtra("subjectID");
		currFileName = sourceIntent.getStringExtra("fileName");
		sourceFileName = currFileName;

		sourceDir = sourceIntent.getStringExtra("fileDir");
		timestamp = sourceIntent.getStringExtra("timeStamp");
		eye = "_L_";

		imgBytes = sourceIntent.getByteArrayExtra("toPass");

		left = sourceIntent.getBooleanExtra("leftEye", true);

		//set text views to passed strings
		idTextView.setText("Subject ID: ");
		idEditText.setText(currId);
		previewLabelTextView.setText("Captured Image:\n" + currFileName);

		// disable the save button until there is a result
		saveButton.setEnabled(false);

		// start segmentation
		new segment().execute(sourceDir, sourceFileName);

		// display the image
		capturedImage = BitmapFactory.decodeFile(sourceDir + currFileName);
		image.setImageBitmap(capturedImage);

		idEditText.addTextChangedListener(new TextWatcher() {

			@Override
			public void afterTextChanged(Editable s) {
				// set the value of subject ID
				currId = s.toString();

				// update the string shown
				updateFilename();
			}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count,
					int after) {
				// unused

			}

			@Override
			public void onTextChanged(CharSequence s, int start, int before,
					int count) {
				// unused

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
		confirmDelete(getCurrentFocus());
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
		newSegFilename = "SEG_" + currId + eye + timestamp;

		// update the textView
		previewLabelTextView.setText("Image Name:\n" + currFileName);

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

		// rename segmented image
		try {
			File oldSegFile = new File(sourceSegDir + sourceSegFilename);
			File newSegFile = new File(newSegDir + newSegFilename);
			oldSegFile.renameTo(newSegFile);
		} catch (Exception e) {
			Log.e("TAG", "Error renaming the file");
			e.printStackTrace();
			makeToast(this, "There was an error renaming the file",
					Toast.LENGTH_LONG);
		}

		// add stuff to intent
		returnIntent.putExtra("subjectID", currId);
		returnIntent.putExtra("fileName", newSegFilename);
		returnIntent.putExtra("leftEye", left);
		returnIntent.putExtra("save", true);

		setResult(0, returnIntent);

		// toast that the image has been saved
		makeToast(this, "The image has been saved!", Toast.LENGTH_LONG);

		finish();
	}

	// onClick for the delete button
	public void deleteButton(View view) {
		delete = true;

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
			File segFile = new File(sourceSegDir + sourceSegFilename);
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

	// confimration pop-up for deleting an image
	public void confirmDelete(final View view) {
		AlertDialog.Builder alert = new AlertDialog.Builder(this);

		alert.setTitle("Delete File");
		alert.setMessage("This can not be undone.");

		alert.setPositiveButton("Delete",
				new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int whichButton) {
						deleteButton(view);
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
		if (!delete) {
			saveButton.setEnabled(true);

			// changes the image displayed from the captured image to the
			// segmented one
			storeImage();

			segmentedImage = BitmapFactory.decodeFile(sourceDir
					+ sourceSegFilename);
			image.setImageBitmap(segmentedImage);

			makeToast(getBaseContext(), "Segmenting Complete!",
					Toast.LENGTH_LONG);
		} else {
			try {
				File segFile = new File(sourceSegDir + sourceSegFilename);
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

	// This is where the segmenting comes in by using JNI to integrate some C++
	// code
	// I have previously written to accept a grayscale BMP and segment the eye
	// NOTE: I did not write the following classes in C++: CLAHE(this is a
	// complicated algorithm
	// used for this iris segmentation that does some histogram equilization
	// and is used by a Haar filter to find the strongest edges in the image.
	// these edges should be the limbic boundary(edge of the iris and sclera -
	// white of the eye) and the pupil boundary between the pupil and iris.
	// From these edges the segmentation then uses a Hough Transform using a
	// directional gradient to find the tangent of all points on the edges
	// if done correctly these tangent lines will all point inward to the
	// center of the pupil from the limbic boundary and pupil boundary
	// where the most points meet on the image SHOULD be the center of the pupil
	// (which would also be the center of the iris).
	// Then the segmentation uses the center to draw concentric circles out
	// growing bigger each time and counting how many points on the circle are
	// touching an edge. Then the circle with the most points on an edge is
	// considered the best fit circle for the eye. These circles are drawn
	// twice: once for the pupil boundary and once for the limbic boundary. The
	// limbic boundary circle is allowed to be much larger than the pupil so
	// that
	// it will create 2 different cirlces instead of finding the same one twice
	// Once the radii of these boundaries are found they would then be used to
	// cut out the iris part of the eye to use later in Recognition (which is
	// not done
	// here. So for now all it does is circles the pupil and iris creating a
	// sort of donut that contains the iris of the eye

	private class segment extends AsyncTask<String, Void, Boolean> {

		// simple conversion of char to string
		public String charToString(char[] array, int start, int last) {
			return new String(array, start, last - start);
		}

		// the pre-execute is just used to tell the user that the device is
		// segmenting their image
		@Override
		protected void onPreExecute() {
			makeToast(getBaseContext(), "Segmenting...", Toast.LENGTH_SHORT);
		}

		// Here is where all the logic happens and the JNI is called
		protected Boolean doInBackground(String... args) {

			args[0] += args[1];
			String pathName = args[0];

			pathName = pathName.substring(0, pathName.lastIndexOf("."));

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
			test.Segment(fileName, path, imgBytes);
			Log.w("test", "post segment");

			return true;
		}

		// this just exits the asynctask
		protected void onPostExecute(Boolean result) {
			segComplete();
		}

	}
}
