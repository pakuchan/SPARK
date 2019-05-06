//////////////////////////////////////////////////////////////////////////////////
// SPARK particle engine														//
// Copyright (C) 2008-2013 - Julien Fryer - julienfryer@gmail.com				//
//																				//
// This software is provided 'as-is', without any express or implied			//
// warranty.  In no event will the authors be held liable for any damages		//
// arising from the use of this software.										//
//																				//
// Permission is granted to anyone to use this software for any purpose,		//
// including commercial applications, and to alter it and redistribute it		//
// freely, subject to the following restrictions:								//
//																				//
// 1. The origin of this software must not be misrepresented; you must not		//
//    claim that you wrote the original software. If you use this software		//
//    in a product, an acknowledgment in the product documentation would be		//
//    appreciated but is not required.											//
// 2. Altered source versions must be plainly marked as such, and must not be	//
//    misrepresented as being the original software.							//
// 3. This notice may not be removed or altered from any source distribution.	//
//////////////////////////////////////////////////////////////////////////////////

#ifndef H_SPK_EMITTERATTACHER
#define H_SPK_EMITTERATTACHER

namespace SPK
{
	class Emitter;

	class SPK_PREFIX EmitterAttacher : public Modifier
	{
	public :

		static Ref<EmitterAttacher> create(
			const Ref<Group>& group = SPK_NULL_REF,
			const Ref<Emitter>& emitter = SPK_NULL_REF,
			bool orientate = false,
			bool rotate = false);

		~EmitterAttacher();

		void setEmitter(const Ref<Emitter>& emitter);
		const Ref<Emitter>& getEmitter() const;

		void setTargetGroup(const Ref<Group>& group);
		const Ref<Group>& getTargetGroup() const;

		void enableEmitterOrientation(bool o);
		void enableEmitterRotation(bool r);
		bool isEmitterOrientationEnabled() const;
		bool isEmitterRotationEnabled() const;

	public :
		spark_description(EmitterAttacher, Modifier)
		(
			spk_attribute(Ref<Emitter>, baseEmitter, setEmitter, getEmitter);
			spk_attribute(Ref<Group>, targetGroup, setTargetGroup, getTargetGroup);
			spk_attribute(bool, enableOrientation, enableEmitterOrientation, isEmitterOrientationEnabled);
			spk_attribute(bool, enableRotation, enableEmitterRotation, isEmitterRotationEnabled);
		);

	protected :

		virtual void createData(DataSet& dataSet,const Group& group) const;
		virtual void checkData(DataSet& dataSet,const Group& group) const;

	private :

		// Data indices
		static const uint32 NB_DATA = 1;
		static const uint32 EMITTER_INDEX = 0;

		class EmitterData : public Data
		{
		public :

			EmitterData(uint32 nbParticles,Group* emittingGroup);
			
			Ref<Emitter>* getEmitters() const;
			void setGroup(Group* group);
			Group* getGroup() const;

			void setEmitter(uint32 index,const Ref<Emitter>& emitter);

		private :

			Ref<Emitter>* data;
			uint32 dataSize;

			Group* group;

			~EmitterData();

			virtual void swap(uint32 index0,uint32 index1);
		};

		Ref<Emitter> baseEmitter;
		Ref<Group> targetGroup;

		bool orientationEnabled;
		bool rotationEnabled;

		EmitterAttacher(
			const Ref<Group>& group = SPK_NULL_REF,
			const Ref<Emitter>& emitter = SPK_NULL_REF,
			bool orientate = false,
			bool rotate = false);

		EmitterAttacher(const EmitterAttacher& emitterAttacher);

		bool checkValidity() const;

		void initData(EmitterData& data,const Group& group) const;

		virtual void init(Particle& particle,DataSet* dataSet) const;
		virtual void modify(Group& group,DataSet* dataSet,float deltaTime) const;
	};

	inline Ref<EmitterAttacher> EmitterAttacher::create(
		const Ref<Group>& group,
		const Ref<Emitter>& emitter,
		bool orientate,
		bool rotate)
	{
		return SPK_NEW(EmitterAttacher,group,emitter,orientate,rotate);
	}

	inline void EmitterAttacher::setEmitter(const Ref<Emitter>& emitter)
	{
		baseEmitter = emitter;
	}

	inline const Ref<Emitter>& EmitterAttacher::getEmitter() const
	{
		return baseEmitter;
	}

	inline void EmitterAttacher::setTargetGroup(const Ref<Group>& group)
	{
		targetGroup = group;
	}

	inline const Ref<Group>& EmitterAttacher::getTargetGroup() const
	{
		return targetGroup;
	}

	inline void EmitterAttacher::enableEmitterRotation(bool rotate)
	{
		rotationEnabled = rotate;
	}

	inline void EmitterAttacher::enableEmitterOrientation(bool orientate)
	{
		orientationEnabled = orientate;
	}

	inline bool EmitterAttacher::isEmitterOrientationEnabled() const
	{
		return orientationEnabled;
	}

	inline bool EmitterAttacher::isEmitterRotationEnabled() const
	{
		return rotationEnabled;
	}

	inline Ref<Emitter>* EmitterAttacher::EmitterData::getEmitters() const
	{
		return data;
	}

	inline void EmitterAttacher::EmitterData::setGroup(Group* group)
	{
		this->group = group;
	}

	inline Group* EmitterAttacher::EmitterData::getGroup() const
	{
		return group;
	}

	inline void EmitterAttacher::EmitterData::swap(uint32 index0,uint32 index1)
	{
		SPK::swap(data[index0],data[index1]); // Calls the optimized swap of Ref instead of the std::swap
	}
}

#endif
