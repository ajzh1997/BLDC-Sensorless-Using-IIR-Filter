/* 
 * File:   IIR_Filter.h
 * Author: Duy Ngo
 *
 * Created on November 23, 2019, 11:16 PM
 */

#ifndef IIR_FILTER_H
#define	IIR_FILTER_H

typedef struct
{
  int  numSectionsLess1;
  int *pCoefs;
  int  psvpage;
  int *pStates;
  int  initialGain;
  int  finalShift;
} IIRCanonicFilter;

typedef struct
{
  int  numSectionsLess1;
  int *pCoefs;
  int  psvpage;
  int *pStates1;
  int *pStates2;
  int  finalShift;
} IIRTransposeFilter;

extern void BlockIIRCanonicFilter( IIRCanonicFilter *, int *, int *, int );
extern void IIRCanonicFilterInit( IIRCanonicFilter *pFilter );

extern void BlockIIRTransposeFilter( IIRTransposeFilter *, int *, int *, int );
extern void IIRTransposeFilterInit( IIRTransposeFilter *);


#endif	/* IIR_FILTER_H */

