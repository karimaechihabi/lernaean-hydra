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
	class SIDX_DLL MovingRegion : public TimeRegion, public IEvolvingShape
	{
        using Region::getLow;
        using Region::getHigh;
        using TimeRegion::intersectsRegionInTime;
        using TimeRegion::containsRegionInTime;
        using TimeRegion::combineRegionInTime;
        using TimeRegion::getCombinedRegionInTime;
        using TimeRegion::containsPointInTime;
        
	public:
		MovingRegion();
		MovingRegion(
			const ts_type* pLow, const ts_type* pHigh,
			const ts_type* pVLow, const ts_type* pVHigh,
			const Tools::IInterval& ti, uint32_t dimension);
		MovingRegion(
			const ts_type* pLow, const ts_type* pHigh,
			const ts_type* pVLow, const ts_type* pVHigh,
			ts_type tStart, ts_type tEnd, uint32_t dimension);
		MovingRegion(
			const Point& low, const Point& high,
			const Point& vlow, const Point& vhigh,
			const Tools::IInterval& ti);
		MovingRegion(
			const Point& low, const Point& high,
			const Point& vlow, const Point& vhigh,
			ts_type tStart, ts_type tEnd);
		MovingRegion(const Region& mbr, const Region& vbr, const Tools::IInterval& ivI);
		MovingRegion(const Region& mbr, const Region& vbr, ts_type tStart, ts_type tEnd);
		MovingRegion(const MovingPoint& low, const MovingPoint& high);
		MovingRegion(const MovingRegion& in);
		virtual ~MovingRegion();

		virtual MovingRegion& operator=(const MovingRegion& r);
		virtual bool operator==(const MovingRegion&) const;

		bool isShrinking() const;

		virtual ts_type getLow(uint32_t index, ts_type t) const;
		virtual ts_type getHigh(uint32_t index, ts_type t) const;
		virtual ts_type getExtrapolatedLow(uint32_t index, ts_type t) const;
		virtual ts_type getExtrapolatedHigh(uint32_t index, ts_type t) const;
		virtual ts_type getVLow(uint32_t index) const;
		virtual ts_type getVHigh(uint32_t index) const;

		virtual bool intersectsRegionInTime(const MovingRegion& r) const;
		virtual bool intersectsRegionInTime(const MovingRegion& r, Tools::IInterval& out) const;
		virtual bool intersectsRegionInTime(const Tools::IInterval& ivI, const MovingRegion& r, Tools::IInterval& ret) const;
		virtual bool containsRegionInTime(const MovingRegion& r) const;
		virtual bool containsRegionInTime(const Tools::IInterval& ivI, const MovingRegion& r) const;
		virtual bool containsRegionAfterTime(ts_type t, const MovingRegion& r) const;

		virtual ts_type getProjectedSurfaceAreaInTime() const;
		virtual ts_type getProjectedSurfaceAreaInTime(const Tools::IInterval& ivI) const;

		virtual ts_type getCenterDistanceInTime(const MovingRegion& r) const;
		virtual ts_type getCenterDistanceInTime(const Tools::IInterval& ivI, const MovingRegion& r) const;

		virtual bool intersectsRegionAtTime(ts_type t, const MovingRegion& r) const;
		virtual bool containsRegionAtTime(ts_type t, const MovingRegion& r) const;

		virtual bool intersectsPointInTime(const MovingPoint& p) const;
		virtual bool intersectsPointInTime(const MovingPoint& p, Tools::IInterval& out) const;
		virtual bool intersectsPointInTime(const Tools::IInterval& ivI, const MovingPoint& p, Tools::IInterval& out) const;
		virtual bool containsPointInTime(const MovingPoint& p) const;
		virtual bool containsPointInTime(const Tools::IInterval& ivI, const MovingPoint& p) const;

		//virtual bool intersectsPointAtTime(ts_type t, const MovingRegion& in) const;
		//virtual bool containsPointAtTime(ts_type t, const MovingRegion& in) const;

		virtual void combineRegionInTime(const MovingRegion& r);
		virtual void combineRegionAfterTime(ts_type t, const MovingRegion& r);
		virtual void getCombinedRegionInTime(MovingRegion& out, const MovingRegion& in) const;
		virtual void getCombinedRegionAfterTime(ts_type t, MovingRegion& out, const MovingRegion& in) const;

		virtual ts_type getIntersectingAreaInTime(const MovingRegion& r) const;
		virtual ts_type getIntersectingAreaInTime(const Tools::IInterval& ivI, const MovingRegion& r) const;

		//
		// IObject interface
		//
		virtual MovingRegion* clone();

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

		//
		// ITimeShape interface
		//
		virtual ts_type getAreaInTime() const;
		virtual ts_type getAreaInTime(const Tools::IInterval& ivI) const;
		virtual ts_type getIntersectingAreaInTime(const ITimeShape& r) const;
		virtual ts_type getIntersectingAreaInTime(const Tools::IInterval& ivI, const ITimeShape& r) const;

		virtual void makeInfinite(uint32_t dimension);
		virtual void makeDimension(uint32_t dimension);

	private:
		void initialize(
			const ts_type* pLow, const ts_type* pHigh,
			const ts_type* pVLow, const ts_type* pVHigh,
			ts_type tStart, ts_type tEnd, uint32_t dimension);

	public:
		class CrossPoint
		{
		public:
			ts_type m_t;
			uint32_t m_dimension;
			uint32_t m_boundary;
			const MovingRegion* m_to;

			struct ascending: public std::binary_function<CrossPoint&, CrossPoint&, bool>
			{
				bool operator()(const CrossPoint& __x, const CrossPoint& __y) const { return __x.m_t > __y.m_t; }
			};
		}; // CrossPoint

	public:
		ts_type* m_pVLow;
		ts_type* m_pVHigh;

		friend SIDX_DLL std::ostream& operator<<(std::ostream& os, const MovingRegion& r);
	}; // MovingRegion

	typedef Tools::PoolPointer<MovingRegion> MovingRegionPtr;
	SIDX_DLL std::ostream& operator<<(std::ostream& os, const MovingRegion& r);
}
