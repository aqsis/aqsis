color eval_ramp(float x; float len; float pos[]; color value[]; string mode)
{
   color blue = color "rgb" (0,0,1);
   return blue;
}

surface rampage()
{

float ramp_f1_pos[3] = {0,0.5,1};
color ramp_f1_col[3] =
   {(1.000000,0.000000,0.000000),(0.000000,1.000000,0.000000),(0.000000,0.000000,1.000000)};


   vector sky = vector(0.494791, 0.500000, 0.500000);
   Ci = eval_ramp(comp(sky, 0),
                  3,
                  ramp_f1_pos, /* <-- Syntax error is on this line. */
                  ramp_f1_col,
                  "linear");
} 
