package com.cse3345.f13.Tanner;

import android.content.Context;
import android.hardware.Camera;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.io.IOException;

public class CameraPreview extends SurfaceView implements
		SurfaceHolder.Callback {
	private SurfaceHolder mHolder;
	private Camera mCamera;

	public CameraPreview(Context context, Camera camera) {
		super(context);
		mCamera = camera;

		// SurfaceHolder.Callback so it notifies when surface is
		// created/destroyed
		mHolder = getHolder();
		mHolder.addCallback(this);
	}

	public void surfaceCreated(SurfaceHolder holder) {
		// The Surface has been created, now tell the camera where to draw the
		// preview.
		try {
			mCamera.setPreviewDisplay(holder);
			mCamera.startPreview();

		} catch (IOException e) {
		}
	}

	public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {

		if (mHolder.getSurface() == null) {
			// preview surface does not exist
			return;
		}

		// stop preview before making changes
		try {
			mCamera.stopPreview();
		} catch (Exception e) {
		}

		// set preview and camera sizes
		Camera.Parameters camParams = mCamera.getParameters();
		camParams.setPreviewSize(640, 480);
		camParams.setPictureSize(640, 480);
		mCamera.setParameters(camParams);

		// start preview with new settings
		try {
			mCamera.setPreviewDisplay(mHolder);
			mCamera.startPreview();

		} catch (Exception e) {
			Log.d("Error", "Error starting camera preview: " + e.getMessage());
		}
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder arg0) {
		// TODO Auto-generated method stub

	}
}
