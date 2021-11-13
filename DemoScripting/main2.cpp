#include <Usagi/Modules/Runtime/Asset/Package/AssetPackageFilesystem.hpp>

#include "interface.hpp"

int main(int argc, char *argv[])
{
    // ========================= Reset data folder ========================== //
    //
    // todo: allow loading the save

    const auto data_folder = "demo_script";
    std::filesystem::remove_all(data_folder);

    App app { data_folder };
    auto &svc = app.services();
    auto &db = app.database_world();

    // Allow SystemInvokeScriptCoroutine to find the script.
    auto &assets = USAGI_SERVICE(svc, ServiceAssetManager);
    assets.add_package(std::make_unique<AssetPackageFilesystem>("."));
#ifdef _DEBUG
    #define PCH_BASE_PATH "x64/Debug/"
#else
    #define PCH_BASE_PATH "x64/Release/"
#endif

    // ================ Init and insert a script entity ===================== //

    Archetype<
        ComponentScript,
        ComponentScriptPCH,
        ComponentCoroutineContinuation,
        ComponentSecondaryAssetRef
    > a;

    // Scripts are built with three steps. First, the definitions of necessary
    // types are preprocessed to a file. This allows building the PCH with only
    // a single file, so that the JIT compiler won't look for header sources all
    // over the disk when validating the PCH. Then, the preprocessed header is
    // compiled into a PCH, which provides information needs to access the
    // entity database during run time. The script is provided with the PCH
    // during JIT compilation. The logic of scene script is a template and is
    // part of the asset. The system invoking the script knows about the actual
    // type of the entity database and appends a snippet which instantiates the
    // script entry function.
    a(C<ComponentScript>()).source_asset_path = "assets/scene.cpp";
    // These are the real paths used to find the _content_ of PCH source and
    // binary. To contain them into asset packages, their name passed to the
    // compiler are remapped and may be different.
    a(C<ComponentScriptPCH>()).src_asset_path = PCH_BASE_PATH "interface.i";
    a(C<ComponentScriptPCH>()).bin_asset_path = PCH_BASE_PATH "interface.pch";
    // This might be different than the asset paths. See CustomBuild step
    // of this project.
    a(C<ComponentScriptPCH>()).source_remapped_name = "db_interface";
    a(C<ComponentCoroutineContinuation>()).resume_condition =
        ComponentCoroutineContinuation::NEXT_FRAME;

    db.insert(a);

    // ========================= Run the game loop ========================== //

    auto &tg = USAGI_SERVICE(app.services(), ServiceStateTransitionGraph);
    while(!tg.should_exit)
    {
        app.update();
    }
}
