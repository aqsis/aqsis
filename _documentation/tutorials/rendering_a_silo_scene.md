---
layout: documentation
title: Rendering a Silo Scene (Basic)
category: tutorial
meta: "The purpose of this tutorial is to go through the steps required for rendering a [Silo](http://www.nevercenter.com/) scene with Aqsis."
order: 20
---

### Introduction

The purpose of this tutorial is to go through the steps required for rendering a [Silo](http://www.nevercenter.com/) scene with Aqsis. 

![]({{ site.baseurl }}/images/silo_basic-interface_abstractdotsia.png)

### Rendering a file

Once happy with the scene rendering can now being…

 * Navigate to Materials/Lights within the main-menu and select the … item from
   the Render option (see below image).

![]({{ site.baseurl }}/images/silo_basic-menu_renderoptions.png)
 
 * Select rib as the ‘Render File Format’, enter aqsis as the ‘Render
   Command/Program’ and -progress as the ‘Render Command Options’ (see below
   image).

![]({{ site.baseurl }}/images/silo_basic-dialog_renderoptions.png)

 * If necessary, select the Options button to change the ‘Render Size’ and
   confirm that ‘Include Lights’, ‘Include Camera’ and ‘Include Materials’ are
   enabled (see below image).

![]({{ site.baseurl }}/images/silo_basic-dialog_exportoptions.png)

 * Select the Render button (see above image), which will result in Aqsis being
   opened and the scene being rendered to the framebuffer (see below image).

![]({{ site.baseurl }}/images/aqsis-render_abstractdotsia.png)

The `Shader “Material0” not found` error is due to the fact Silo maps material
names to RenderMan shaders, and there is no Material0 shader by default.

You can use Silo’s Material Editor to change the name of this material to a
shader that does exist, like plastic (see below image).

![]({{ site.baseurl }}/images/silo_basic-dialog_materialeditor.png)

 * Re-rendering the scene will now result the plastic shader being used instead
   (see below image).

![]({{ site.baseurl }}/images/aqsis-render_abstractdotsia2.png)

|---
| Note
|---
| Make sure to press the Enter key after entering a material name, otherwise the changes will not take effect !!!
|---

<br/>

 * Once mastering the material/shaders mapping system a whole host of visual
   effects can be achieved (see below images).

![]({{ site.baseurl }}/images/aqsis-render_abstractdotsia5.png)
 
![]({{ site.baseurl }}/images/aqsis-render_abstractdotsia6.png)
 
![]({{ site.baseurl }}/images/aqsis-render_abstractdotsia7.png)


Above shaders courtesy of the [Advanced RenderMan
book](http://books.elsevier.com/bookscat/links/details.asp?isbn=1558606181), by
Anthony A. Apodaca and Larry Gritz.
