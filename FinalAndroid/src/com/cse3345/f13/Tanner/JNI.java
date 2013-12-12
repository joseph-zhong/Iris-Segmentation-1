package com.cse3345.f13.Tanner;

public class JNI {

	public native void Segment(String fileName, String path, byte[] passed);

	static {
		System.loadLibrary("AndroidSegmentor");
	}

	public static void main(String[] args) {
		//No need to run anything as it calls the library and then just returns
	}

}