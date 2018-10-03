/*
 * Face Masks for SlOBS
 * Copyright (C) 2017 General Workings Inc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */
#pragma once
#include "mask-resource.h"
#include "mask-resource-model.h"

namespace Mask {
	namespace Resource {

		class Emitter;

		class Particle : public SortedDrawObject {
		public:
			enum State : uint8_t {
				DEAD,
				SPAWNED,
				ALIVE,
				NUM_STATES
			};

			std::size_t		id;
			vec3			position;
			vec3			velocity;
			Emitter*		emitter;
			float			elapsed;
			State			state;

			Particle* next;

			Particle() : state(DEAD), emitter(nullptr) {}

			virtual float	SortDepth() override;
			virtual void	SortedRender() override;
		};

		struct EmitterInstanceData : public InstanceData {
			Particle*	particles;
			int			numParticles;
			float		elapsed;
			float		delta_time;

			EmitterInstanceData() : particles(nullptr), 
				numParticles(0), elapsed(0.0f), delta_time(0.0f) {}

			inline void Init(int num_particles, Emitter* e) {
				numParticles = num_particles;
				// only init once
				if (particles != nullptr)
					return;
				particles = new Particle[numParticles];
				Particle* p = particles;
				std::hash<int> hasher;
				for (int i = 0; i < numParticles; i++,p++) {
					p->id = hasher(i);
					p->emitter = e;
				}
			}

			void Reset() override {
				elapsed = 0.0f;
				if (particles == nullptr)
					return;
				Particle* p = particles;
				for (int i = 0; i < numParticles; i++, p++) {
					p->state = Particle::DEAD;
				}
			}

			~EmitterInstanceData() {
				if (particles) {
					delete[] particles;
				}
			}
		};

		class Emitter : public IBase {
		public:
			Emitter(Mask::MaskData* parent, std::string name, obs_data_t* data);
			virtual ~Emitter();

			virtual Type GetType() override;
			virtual void Update(Mask::Part* part, float time) override;
			virtual void Render(Mask::Part* part) override;
			virtual bool IsDepthOnly() override;
			
			bool IsOpaque();

		protected:
			// allow Particle class to access protected vars
			friend class Particle;

			float		m_rateMin, m_rateMax;
			float		m_lifetime;
			float		m_frictionMin, m_frictionMax;
			float		m_alphaStart, m_alphaEnd;
			float		m_scaleStart, m_scaleEnd;
			vec3		m_forceMin, m_forceMax;
			vec3		m_initialVelocityMin, m_initialVelocityMax;
			int			m_numParticles;
			float		m_zSortOffset;
			bool		m_worldSpace;
			bool		m_inverseRate;
			std::shared_ptr<Model> m_model;

			static	float RandFloat(float min, float max);
		};
	}
}
