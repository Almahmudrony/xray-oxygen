#ifndef _MManipData
#define _MManipData
//-
// ==========================================================================
// Copyright (C) 1995 - 2006 Autodesk, Inc., and/or its licensors.  All
// rights reserved.
//
// The coded instructions, statements, computer programs, and/or related
// material (collectively the "Data") in these files contain unpublished
// information proprietary to Autodesk, Inc. ("Autodesk") and/or its
// licensors,  which is protected by U.S. and Canadian federal copyright law
// and by international treaties.
//
// The Data may not be disclosed or distributed to third parties or be
// copied or duplicated, in whole or in part, without the prior written
// consent of Autodesk.
//
// The copyright notices in the Software and this entire statement,
// including the above license grant, this restriction and the following
// disclaimer, must be included in all copies of the Software, in whole
// or in part, and all derivative works of the Software, unless such copies
// or derivative works are solely in the form of machine-executable object
// code generated by a source language processor.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
// AUTODESK DOES NOT MAKE AND HEREBY DISCLAIMS ANY EXPRESS OR IMPLIED
// WARRANTIES INCLUDING, BUT NOT LIMITED TO, THE WARRANTIES OF
// NON-INFRINGEMENT, MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE,
// OR ARISING FROM A COURSE OF DEALING, USAGE, OR TRADE PRACTICE. IN NO
// EVENT WILL AUTODESK AND/OR ITS LICENSORS BE LIABLE FOR ANY LOST
// REVENUES, DATA, OR PROFITS, OR SPECIAL, DIRECT, INDIRECT, OR
// CONSEQUENTIAL DAMAGES, EVEN IF AUTODESK AND/OR ITS LICENSORS HAS
// BEEN ADVISED OF THE POSSIBILITY OR PROBABILITY OF SUCH DAMAGES.
// ==========================================================================
//+
//
// CLASS:    MManipData
//
// ****************************************************************************
//
#if defined __cplusplus

// ****************************************************************************
// INCLUDED HEADER FILES


#include <maya/MStatus.h>
#include <maya/MObject.h>

// ****************************************************************************
// CLASS DECLARATION (MManipData)

//! \ingroup OpenMayaUI
//! \brief Manipulator Data 
/*!
This class encapulates manipulator data which is returned from the
manipulator conversion functions.  MManipData is used to represent
data that is either simple or complex.  Simple data is used to
represent bool, short, int, unsigned int, float, and double types.
Complex data is used to represent MObjects created by MFnData, or
classes derived from MFnData.
*/
class OPENMAYAUI_EXPORT MManipData
{
public:
	MManipData();
	~MManipData();
	MManipData(const MManipData &);
	MManipData(bool);
	MManipData(short);
	MManipData(int);
	MManipData(unsigned int);
	MManipData(float);
	MManipData(double);
	MManipData(const MObject &);

	MManipData &	operator=(const MManipData &);
	MManipData &	operator=(bool);
	MManipData &	operator=(short);
	MManipData &	operator=(int);
	MManipData &	operator=(unsigned int);
	MManipData &	operator=(float);
	MManipData &	operator=(double);
	MManipData &	operator=(const MObject &);

	bool			isSimple()		const;

	bool			asBool()		const;
	short			asShort()		const;
	int			asLong()		const;
	unsigned int		asUnsigned()	const;
	float			asFloat()		const;
	double			asDouble()		const;
	MObject			asMObject()		const;

	static const char*	className();

private:
	bool		fIsSimple;
	double		fSimpleData;
	MObject		fComplexData;
};

#endif /* __cplusplus */
#endif /* _MManipData */
