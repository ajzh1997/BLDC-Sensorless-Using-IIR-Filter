/* 
 * File:   snap.h
 * Author: Duy Ngo
 *
 * Created on November 24, 2019, 11:01 AM
 */

#ifndef SNAP_H
#define	SNAP_H

//#define SNAPSHOT

#ifdef SNAPSHOT
int pha_Raw;
int pha_pre_filtered;
int pha_filtered;
int signal_average_uart;
#endif

#endif	/* SNAP_H */

