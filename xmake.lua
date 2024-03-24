target("marl")
_config_project({
    project_kind = "shared"
})
add_includedirs("include", {
    public = true
})
if is_plat("windows") then
    add_defines("NOMINMAX")
end
add_files("src/*.c")
add_defines("MARL_BUILDING_DLL")
add_defines("MARL_DLL", {public = true})
add_deps("eastl")
on_load(function(target)
    local src_path = path.join(os.scriptdir(), "src")
    for _, filepath in ipairs(os.files(path.join(src_path, "*.cpp"))) do
        local file_name = path.filename(filepath)
        if not (file_name:find("_bench") or file_name:find("_test")) then
            target:add("files", filepath)
        end
    end
end)
target_end()
