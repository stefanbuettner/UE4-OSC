using System.IO;

namespace UnrealBuildTool.Rules
{
	public class SSI : ModuleRules
	{
		public SSI(ReadOnlyTargetRules Target) : base(Target)
        {
			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Networking",
				}
			); 

			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"Core",
					"CoreUObject",
                    "Engine",
					"Sockets",
                    "InputCore",
                    "Slate",
                    "SlateCore",
				}
			);

			PrivateIncludePathModuleNames.AddRange(
				new string[] {
					"Settings",
				}
			);

            PublicIncludePaths.AddRange(
				new string[] {
					Path.Combine(ModuleDirectory, "Public"),
					Path.Combine(ModuleDirectory, "Public/Common"),
				}
            );

			PrivateIncludePaths.AddRange(
				new string[] {
					Path.Combine(ModuleDirectory, "Private"),
					Path.Combine(ModuleDirectory, "Private/Common"),
				}
			);

            if (Target.Type == TargetRules.TargetType.Editor)
            {
                PublicDefinitions.Add("SSI_EDITOR_BUILD=1");

                PrivateDependencyModuleNames.Add("UnrealEd");
            }
            else
            {
                PublicDefinitions.Add("SSI_EDITOR_BUILD=0");
            }
        }
	}
}
