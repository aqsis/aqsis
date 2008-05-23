/******************************************************************************/
/* COPYRIGHT                                                                  */
/*                                                                            */
/* Copyright 2000 by Schroff Development Corporation, Shawnee-Mission,        */
/* Kansas, United States of America. All rights reserved.                     */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* This Display Driver is distributed as "freeware". There are no             */
/* restrictions on its' usage.                                                */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* DISCLAIMER OF ALL WARRANTIES AND LIABILITY                                 */
/*                                                                            */
/* Schroff Development Corporation makes no warranties, either expressed      */
/* or implied, with respect to the programs contained within this file, or    */
/* with respect to software used in conjunction with these programs. The      */
/* programs are distributed 'as is'.  The entire risk as to its quality and   */
/* performance is assumed by the purchaser.  Schroff  Development Corporation */
/* does not guarantee, warrant or make any representation regarding the       */
/* use of, or the results of the use of the programs in terms of correctness, */
/* accuracy, reliability, or performance. Schroff Development Corporation     */
/* assumes no liability for any direct, indirect, or consquential, special    */
/* or exemplary damages, regardless of its having been advised of the         */
/* possibility of such damage.                                                */
/*                                                                            */
/******************************************************************************/


//
// This is a Display Driver that was written to comply with the PhotoRealistic
// RenderMan Display Driver Implementation Guide (on the web at:
// www.pixar.com/products/rendermandocs/toolkit/Toolkit/dspy.html).
//
// This driver places image data into a Windows .BMP file. It writes this data
// one scanline at a time and tries to minimize memory consumption.
//


