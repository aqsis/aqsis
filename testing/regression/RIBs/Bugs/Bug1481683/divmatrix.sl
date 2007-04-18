surface divmatrix(string shadName="";)
{
    /* A transformation matrix that transforms points from "current" space to 
     * the shadows "camera" space */ 
    uniform matrix CurrentToShadowCamSpace = 1;

    /* A transformation matrix that transforms points from "current" space to 
     * the shadows "parallel projection" space */
    uniform matrix CurrentToShadowProjSpace = 1;

    /* Query shadow map for shadow's camera transformation matrix */
    if (shadName != "")
    	textureinfo(shadName, "viewingmatrix", CurrentToShadowCamSpace);

    /* Calculate the inverse of the shadow camera transformation matrix */
    uniform matrix InvCurrentToShadowCamSpace = 1/CurrentToShadowCamSpace;

Oi = 1.0;
Ci = noise(s,t);
}

