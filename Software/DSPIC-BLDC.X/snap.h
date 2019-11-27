/* 
 * File:   snap.h
 * Author: Duy Ngo
 *
 * Created on November 24, 2019, 11:01 AM
 */

#ifndef SNAP_H
#define	SNAP_H



#ifdef SNAPSHOT
	#define SNAPSIZE 64   // this number must be 2 to some power x  (i.e. 8, 16, 1024, 256) MASKING will be messed up if you don't do this
extern int SnapBuf1[SNAPSIZE];
extern int SnapBuf2[SNAPSIZE];
extern int SnapBuf3[SNAPSIZE];
extern int SnapBuf4[SNAPSIZE];
#endif

#endif	/* SNAP_H */

