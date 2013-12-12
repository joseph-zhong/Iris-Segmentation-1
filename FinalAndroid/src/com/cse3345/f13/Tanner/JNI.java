package com.cse3345.f13.Tanner;


public class JNI {

	public native void Segment(String fileName, String path, Eye stats, byte[] passed);

	static {
		System.loadLibrary("AndroidSegmentor");
	}

	public static void main(String[] args) {
	}

}