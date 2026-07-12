const std = @import("std");

pub fn build(b: *std.Build) void {
    // 1. Force the engine to target a pure x86_64 freestanding bare-metal profile
    const query = std.Target.Query{
        .cpu_arch = .x86_64,
        .os_tag = .freestanding,
        .abi = .none,
    };
    const target = b.resolveTargetQuery(query);
    const optimize = b.standardOptimizeOption(.{});

    // 2. Instantiate our compilation artifact container
    const kernel = b.addExecutable(.{
        .name = "kernel.elf",
        .target = target,
        .optimize = optimize,
    });

    // 3. Mount our custom layout configurations and files
    kernel.addCSourceFile(.{
        .file = b.path("main.c"),
        .flags = &.{ 
            "-std=c11", 
            "-ffreestanding", 
            "-fno-stack-protector", 
            "-fno-pic", 
            "-mno-red-zone" 
        },
    });

    // 4. Inject our customized hardware linker alignment map layout rules
    kernel.setLinkerScript(b.path("linker.ld"));

    // 5. Enforce specialized system processing parameters
    kernel.root_module.red_zone = false;
    
    // 6. Build target file artifact generation output pipeline
    b.installArtifact(kernel);
}
