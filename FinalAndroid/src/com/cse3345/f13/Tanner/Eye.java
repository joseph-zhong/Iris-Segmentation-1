package com.cse3345.f13.Tanner;

public class Eye {
	double pupilRad;
	double limbicRad;
	int centerX;
	int centerY;

	public Eye() {
		pupilRad = 0.0;
		limbicRad = 0.0;
		centerX = 0;
		centerY = 0;
	}

	public void setStats(double pupil, double limbic, int x, int y) {

		pupilRad = pupil;
		limbicRad = limbic;
		centerX = x;
		centerY = y;
	}

	public double getPupil() {

		return pupilRad;
	}

	public double getLimbic() {

		return limbicRad;
	}

	public int getX() {

		return centerX;
	}

	public int getY() {

		return centerY;
	}
}
