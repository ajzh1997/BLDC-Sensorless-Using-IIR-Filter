/* 
 * File:   BEMF_filter.h
 * Author: Duy Ngo
 *
 * Created on November 23, 2019, 11:16 PM
 */

#ifndef BEMF_FILTER_H
#define	BEMF_FILTER_H

extern IIRTransposeFilter BEMF_phaseA_Filter;
extern IIRTransposeFilter BEMF_phaseB_Filter;
extern IIRTransposeFilter BEMF_phaseC_Filter;
extern unsigned int BEMF_filterCoefs_81940Hz;
extern unsigned int BEMF_filterCoefs_49152Hz;

#endif	/* BEMF_FILTER_H */

