/*
 * imageralphabug.sl
 *
 * Imager shader to test overlaying a "watermark" on an image.
 * Currently, this does not work for RGBA rendering, only
 * for RGB rendering. The output alpha is left at the value sent
 * as Oi (or alpha?) to the imager shader, not the Oi output from
 * this code.
 *
 * Author: Stefan Gustavson (stegu@itn.liu.se), 2006-01-22
 */
imager imageralphabug() {
  float resolution[3];
  option("Format", resolution);
  float width = resolution[0];
  float height = resolution[1];
  float s = xcomp(P) / width;
  float t = ycomp(P) / height;
  float overlayalpha = 0.5 * min(step(0.5,s), step(0.5,t));
  color overlaycolor = noise(s*10, t*10); // Simple test pattern
  
  // This incorporates "overlaycolor" as a subtractive semi-transparent overlay
  Ci = Ci*(1-overlayalpha) + overlaycolor*overlayalpha; // Blend in the color
  Oi = 1-(1-Oi)*(1-overlayalpha); // Correct subtractive treatment of Oi
}
