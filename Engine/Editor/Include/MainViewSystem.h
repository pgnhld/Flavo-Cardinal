#pragma once

#include "Global.h"
#include "EECS.h"
#include "SystemManagementSystem.h"

namespace framework {
	enum class BackgroundMusicType;
}

namespace ft_editor
{
	class MainViewSystem : public eecs::System<MainViewSystem>, public eecs::IInvoker
	{
	public:
		MainViewSystem();
		MainViewSystem(const MainViewSystem& another) = delete;
		MainViewSystem(MainViewSystem&& another) = delete;
		~MainViewSystem();
		MainViewSystem& operator=(const MainViewSystem& another) = delete;
		MainViewSystem& operator=(MainViewSystem&& another) = delete;

		void update(eecs::EntityManager& entities, double deltaTime) override;

	private:
		void open() const;
		void save();
		void saveAs();
		void play() const;
		void refresh() const;
		void exit() const;
		void playMusic(framework::BackgroundMusicType musicType) const;

		void saveEntityRecursive(ft_engine::Transform* transform, int level, uint64 parentId, std::ostream& stream);
	};
}
