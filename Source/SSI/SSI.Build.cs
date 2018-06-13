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
					"SSI/Public",
					"SSI/Public/Common",
					"SSI/Public/Receive",
					"SSI/Public/Send",
				}
            );

			PrivateIncludePaths.AddRange(
				new string[] {
					"SSI/Private",
					"SSI/Private/Common",
					"SSI/Private/Receive",
					"SSI/Private/Send",
				}
			);

            if (Target.Type == TargetRules.TargetType.Editor)
            {
                Definitions.Add("SSI_EDITOR_BUILD=1");

                PrivateDependencyModuleNames.Add("UnrealEd");
            }
            else
            {
                Definitions.Add("SSI_EDITOR_BUILD=0");
            }
        }
	}
}
