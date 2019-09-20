/******************************************************************************
 * Project:  libspatialindex - A C++ library for spatial indexing
 * Author:   Marios Hadjieleftheriou, mhadji@gmail.com
 ******************************************************************************
 * Copyright (c) 2003, Marios Hadjieleftheriou
 *
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
******************************************************************************/

#pragma once

namespace SpatialIndex
{
	class SIDX_DLL MovingPoint : public TimePoint, public IEvolvingShape
	{
	public:
		MovingPoint();
		MovingPoint(const ts_type* pCoords, const ts_type* pVCoords, const Tools::IInterval& ti, uint32_t dimension);
		MovingPoint(const ts_type* pCoords, const ts_type* pVCoords, ts_type tStart, ts_type tEnd, uint32_t dimension);
		MovingPoint(const Point& p, const Point& vp, const Tools::IInterval& ti);
		MovingPoint(const Point& p, const Point& vp, ts_type tStart, ts_type tEnd);
		MovingPoint(const MovingPoint& p);
		virtual ~MovingPoint();

		virtual MovingPoint& operator=(const MovingPoint& p);
		virtual bool operator==(const MovingPoint& p) const;

		virtual ts_type getCoord(uint32_t index, ts_type t) const;
		virtual ts_type getProjectedCoord(uint32_t index, ts_type t) const;
		virtual ts_type getVCoord(uint32_t index) const;
		virtual void getPointAtTime(ts_type t, Point& out) const;

		//
		// IObject interface
		//
		virtual MovingPoint* clone();

		//
		// ISerializable interface
		//
		virtual uint32_t getByteArraySize();
		virtual void loadFromByteArray(const byte* data);
		virtual void storeToByteArray(byte** data, uint32_t& len);

		//
		// IEvolvingShape interface
		//
		virtual void getVMBR(Region& out) const;
		virtual void getMBRAtTime(ts_type t, Region& out) const;

		virtual void makeInfinite(uint32_t dimension);
		virtual void makeDimension(uint32_t dimension);

	private:
		void initialize(
			const ts_type* pCoords, const ts_type* pVCoords,
			ts_type tStart, ts_type tEnd, uint32_t dimension);

	public:
		ts_type* m_pVCoords;

		friend SIDX_DLL std::ostream& operator<<(std::ostream& os, const MovingPoint& pt);
	}; // MovingPoint

	SIDX_DLL std::ostream& operator<<(std::ostream& os, const MovingPoint& pt);
}

