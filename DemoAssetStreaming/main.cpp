
#include <future>
#include <iostream>

#include <Usagi/Modules/Runtime/Asset/AssetManager.hpp>
#include <Usagi/Modules/Runtime/Asset/Package/AssetPackageFilesystem.hpp>

using namespace usagi;

class StdTaskExecutor : public TaskExecutor
{
public:
    void submit(std::unique_ptr<Task> task) override
    {
        std::thread([t = std::move(task)]() {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(5ms);
            if(!t->precondition()) throw std::runtime_error("");
            t->on_started();
            t->run();
            t->on_finished();
            if(!t->postcondition()) throw std::runtime_error("");
        }).detach();
    }
};

int main(int argc, char *argv[])
{
    StdTaskExecutor executor;
    AssetManager asset_manager;
    asset_manager.add_package(std::make_shared<AssetPackageFilesystem>(""));

    for(int i = 0; i < 32; ++i)
    {
        const auto future = std::async(
            std::launch::async, [&]() {
            PrimaryAsset asset;
            do
            {
                asset = asset_manager.primary_asset(argv[1], &executor);
                switch(asset.status)
                {
                    case AssetStatus::PRIMARY_FOUND:
                        std::cout << "Found" << std::endl;
                        break;
                    case AssetStatus::PRIMARY_PENDING:
                        std::cout << "Pending" << std::endl;
                        break;
                    case AssetStatus::PRIMARY_LOADING:
                        std::cout << "Loading" << std::endl;
                        break;
                    case AssetStatus::PRIMARY_READY:
                        std::cout << "Ready" << std::endl;
                        break;
                    default:
                        throw 0;
                }
            } while(asset.status != AssetStatus::PRIMARY_READY);
        });
    }
}
