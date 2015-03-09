// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "LocalizationDashboardPrivatePCH.h"
#include "LocalizationConfigurationScript.h"
#include "Classes/LocalizationTarget.h"

namespace LocalizationConfigurationScript
{
	FString MakePathRelativeToProjectDirectory(const FString& Path)
	{
		FString Result = Path;
		FPaths::MakePathRelativeTo(Result, *FPaths::GameDir());
		return Result;
	}

	FString GetScriptDirectory()
	{
		return FPaths::GameConfigDir() / TEXT("Localization");
	}

	FString GetDataDirectory(const ULocalizationTarget* const Target)
	{
		return FPaths::GameContentDir() / TEXT("Localization") / Target->Settings.Name;
	}

	TArray<FString> GetScriptPaths(const ULocalizationTarget* const Target)
	{
		TArray<FString> Result;
		Result.Add(GetGatherScriptPath(Target));
		Result.Add(GetImportScriptPath(Target));
		Result.Add(GetExportScriptPath(Target));
		Result.Add(GetReportScriptPath(Target));
		return Result;
	}

	FString GetManifestPath(const ULocalizationTarget* const Target)
	{
		return GetDataDirectory(Target) / FString::Printf( TEXT("%s.%s"), *Target->Settings.Name, TEXT("manifest") );
	}

	FString GetArchivePath(const ULocalizationTarget* const Target, const FString& CultureName)
	{
		return GetDataDirectory(Target) / CultureName / FString::Printf( TEXT("%s.%s"), *Target->Settings.Name, TEXT("archive") );
	}

	FString GetDefaultPOFileName(const ULocalizationTarget* const Target)
	{
		return FString::Printf( TEXT("%s.%s"), *Target->Settings.Name, TEXT("po") );
	}

	FString GetDefaultPOPath(const ULocalizationTarget* const Target, const FString& CultureName)
	{
		return GetDataDirectory(Target) / CultureName / GetDefaultPOFileName(Target);
	}

	FString GetLocResPath(const ULocalizationTarget* const Target, const FString& CultureName)
	{
		return GetDataDirectory(Target) / CultureName / FString::Printf( TEXT("%s.%s"), *Target->Settings.Name, TEXT("locres") );
	}

	FString GetWordCountCSVPath(const ULocalizationTarget* const Target)
	{
		return GetDataDirectory(Target) / FString::Printf( TEXT("%s.%s"), *Target->Settings.Name, TEXT("csv") );
	}

	FString GetConflictReportPath(const ULocalizationTarget* const Target)
	{
		return GetDataDirectory(Target) / FString::Printf( TEXT("%s_Conflicts.%s"), *Target->Settings.Name, TEXT("txt") );
	}

	FLocalizationConfigurationScript GenerateGatherScript(const ULocalizationTarget* const Target)
	{
		FLocalizationConfigurationScript Script;

		const bool HasSourceCode = IFileManager::Get().DirectoryExists( *FPaths::GameSourceDir() );

		const FString ConfigDirRelativeToGameDir = MakePathRelativeToProjectDirectory(FPaths::GameConfigDir());
		const FString SourceDirRelativeToGameDir = MakePathRelativeToProjectDirectory(FPaths::GameSourceDir());
		const FString ContentDirRelativeToGameDir = MakePathRelativeToProjectDirectory(FPaths::GameContentDir());

		// CommonSettings
		{
			FConfigSection& ConfigSection = Script.CommonSettings();

			const ULocalizationTargetSet* const LocalizationTargetSet = GetDefault<ULocalizationTargetSet>(ULocalizationTargetSet::StaticClass());
			for (const FString& TargetDependencyName : Target->Settings.TargetDependencies)
			{
				ULocalizationTarget* const * OtherTarget = LocalizationTargetSet->TargetObjects.FindByPredicate([&TargetDependencyName](ULocalizationTarget* const OtherTarget)->bool{return OtherTarget->Settings.Name == TargetDependencyName;});
				if (OtherTarget)
				{
					ConfigSection.Add( TEXT("ManifestDependencies"), MakePathRelativeToProjectDirectory(GetManifestPath(*OtherTarget)) );
				}
			}

			for (const FFilePath& Path : Target->Settings.AdditionalManifestDependencies)
			{
				ConfigSection.Add( TEXT("ManifestDependencies"), MakePathRelativeToProjectDirectory(Path.FilePath) );
			}

			const FString SourcePath = ContentDirRelativeToGameDir / TEXT("Localization") / Target->Settings.Name;
			ConfigSection.Add( TEXT("SourcePath"), SourcePath );
			const FString DestinationPath = ContentDirRelativeToGameDir / TEXT("Localization") / Target->Settings.Name;
			ConfigSection.Add( TEXT("DestinationPath"), DestinationPath );

			ConfigSection.Add( TEXT("ManifestName"), FPaths::GetCleanFilename(GetManifestPath(Target)) );
			ConfigSection.Add( TEXT("ArchiveName"), FPaths::GetCleanFilename(GetArchivePath(Target, FString())) );

			ConfigSection.Add( TEXT("SourceCulture"), Target->Settings.NativeCultureStatistics.CultureName );
			ConfigSection.Add( TEXT("CulturesToGenerate"), Target->Settings.NativeCultureStatistics.CultureName );
			for (const FCultureStatistics& CultureStatistics : Target->Settings.SupportedCulturesStatistics)
			{
				ConfigSection.Add( TEXT("CulturesToGenerate"), CultureStatistics.CultureName );
			}
		}

		uint32 GatherTextStepIndex = 0;
		// GatherTextFromSource
		if (Target->Settings.GatherFromTextFiles.SearchDirectories.Num() && Target->Settings.GatherFromTextFiles.FileExtensions.Num()) // Don't gather from source if there are no source files.
		{
			FConfigSection& ConfigSection = Script.GatherTextStep(GatherTextStepIndex++);

			// CommandletClass
			ConfigSection.Add( TEXT("CommandletClass"), TEXT("GatherTextFromSource") );

			// Include Paths
			for (const auto& IncludePath : Target->Settings.GatherFromTextFiles.SearchDirectories)
			{
				ConfigSection.Add( TEXT("IncludePaths"), IncludePath );
			}

			// Exclude Paths
			ConfigSection.Add( TEXT("ExcludePaths"), ConfigDirRelativeToGameDir / TEXT("Localization") );
			for (const auto& ExcludePath : Target->Settings.GatherFromTextFiles.ExcludePathWildcards)
			{
				ConfigSection.Add( TEXT("ExcludePaths"), ExcludePath );
			}

			// Source File Search Filters
			for (const auto& FileExtension : Target->Settings.GatherFromTextFiles.FileExtensions)
			{
				ConfigSection.Add( TEXT("SourceFileSearchFilters"), FileExtension );
			}
		}

		// GatherTextFromAssets
		if (Target->Settings.GatherFromTextFiles.SearchDirectories.Num() && Target->Settings.GatherFromTextFiles.FileExtensions.Num()) // Don't gather from packages if there are none specified.
		{
			FConfigSection& ConfigSection = Script.GatherTextStep(GatherTextStepIndex++);

			// CommandletClass
			ConfigSection.Add( TEXT("CommandletClass"), TEXT("GatherTextFromAssets") );

			// Include Paths
			for (const auto& IncludePath : Target->Settings.GatherFromPackages.IncludePathWildcards)
			{
				ConfigSection.Add( TEXT("IncludePaths"), IncludePath );
			}

			// Exclude Paths
			ConfigSection.Add( TEXT("ExcludePaths"), ContentDirRelativeToGameDir / TEXT("Localization") );
			for (const auto& ExcludePath : Target->Settings.GatherFromPackages.ExcludePathWildcards)
			{
				ConfigSection.Add( TEXT("ExcludePaths"), ExcludePath );
			}

			// Package Extensions
			for (const auto& FileExtension : Target->Settings.GatherFromPackages.FileExtensions)
			{
				ConfigSection.Add( TEXT("PackageExtensions"), FileExtension );
			}
		}

		// GatherTextFromMetadata
		if (Target->Settings.GatherFromMetaData.IncludePathWildcards.Num() && Target->Settings.GatherFromMetaData.KeySpecifications.Num()) // Don't gather from metadata if there are none specified.
		{
			FConfigSection& ConfigSection = Script.GatherTextStep(GatherTextStepIndex++);

			// CommandletClass
			ConfigSection.Add( TEXT("CommandletClass"), TEXT("GatherTextFromMetadata") );

			// Include Paths
			for (const auto& IncludePath : Target->Settings.GatherFromMetaData.IncludePathWildcards)
			{
				ConfigSection.Add( TEXT("IncludePaths"), IncludePath );
			}

			// Exclude Paths
			for (const auto& ExcludePath : Target->Settings.GatherFromMetaData.ExcludePathWildcards)
			{
				ConfigSection.Add( TEXT("ExcludePaths"), ExcludePath );
			}

			// Package Extensions
			for (const FMetaDataKeyGatherSpecification& Specification : Target->Settings.GatherFromMetaData.KeySpecifications)
			{
				ConfigSection.Add( TEXT("InputKeys"), Specification.MetaDataKey );
				ConfigSection.Add( TEXT("OutputNamespaces"), Specification.TextNamespace );
				ConfigSection.Add( TEXT("OutputKeys"), FString::Printf(TEXT("\"%s\""), *Specification.TextKeyPattern) );
			}
		}

		// GenerateGatherManifest
		{
			FConfigSection& ConfigSection = Script.GatherTextStep(GatherTextStepIndex++);

			// CommandletClass
			ConfigSection.Add( TEXT("CommandletClass"), TEXT("GenerateGatherManifest") );
		}

		// GenerateGatherArchive
		{
			FConfigSection& ConfigSection = Script.GatherTextStep(GatherTextStepIndex++);

			// CommandletClass
			ConfigSection.Add( TEXT("CommandletClass"), TEXT("GenerateGatherArchive") );
		}

		Script.Dirty = true;

		return Script;
	}

	FString GetGatherScriptPath(const ULocalizationTarget* const Target)
	{
		return GetScriptDirectory() / FString::Printf( TEXT("%s_Gather.%s"), *(Target->Settings.Name), TEXT("ini") );
	}

	FLocalizationConfigurationScript GenerateImportScript(const ULocalizationTarget* const Target, const TOptional<FString> CultureName, const TOptional<FString> OutputPathOverride)
	{
		FLocalizationConfigurationScript Script;

		const FString ContentDirRelativeToGameDir = MakePathRelativeToProjectDirectory(FPaths::GameContentDir());

		// GatherTextStep0 - InternationalizationExport
		{
			FConfigSection& ConfigSection = Script.GatherTextStep(0);

			// CommandletClass
			ConfigSection.Add( TEXT("CommandletClass"), TEXT("InternationalizationExport") );

			ConfigSection.Add( TEXT("bImportLoc"), TEXT("true") );

			FString SourcePath;
			// Overriding output path changes the source directory for the PO file.
			if (OutputPathOverride.IsSet())
			{
				// The output path for a specific culture is a file path.
				if (CultureName.IsSet())
				{
					SourcePath = MakePathRelativeToProjectDirectory( FPaths::GetPath(OutputPathOverride.GetValue()) );
				}
				// Otherwise, it is a directory path.
				else
				{
					SourcePath = MakePathRelativeToProjectDirectory( OutputPathOverride.GetValue() );
				}
			}
			// Use the default PO file's directory path.
			else
			{
				SourcePath = ContentDirRelativeToGameDir / TEXT("Localization") / Target->Settings.Name;
			}
			ConfigSection.Add( TEXT("SourcePath"), SourcePath );

			const FString DestinationPath = ContentDirRelativeToGameDir / TEXT("Localization") / Target->Settings.Name;
			ConfigSection.Add( TEXT("DestinationPath"), DestinationPath );

			const auto& AddCultureToGenerate = [&](const int32 Index)
			{
				ConfigSection.Add( TEXT("CulturesToGenerate"), Target->Settings.SupportedCulturesStatistics[Index].CultureName );
			};

			// Export for a specific culture.
			if (CultureName.IsSet())
			{
				ConfigSection.Add( TEXT("CulturesToGenerate"), CultureName.GetValue() );
			}
			// Export for all cultures.
			else
			{
				for (const FCultureStatistics& CultureStatistics : Target->Settings.SupportedCulturesStatistics)
				{
					ConfigSection.Add( TEXT("CulturesToGenerate"), CultureStatistics.CultureName );
				}
			}

			// Do not use culture subdirectories if importing a single culture to a specific directory.
			if (CultureName.IsSet() && OutputPathOverride.IsSet())
			{
				ConfigSection.Add( TEXT("bUseCultureDirectory"), "false" );
			}

			ConfigSection.Add( TEXT("ManifestName"), FPaths::GetCleanFilename(GetManifestPath(Target)) );
			ConfigSection.Add( TEXT("ArchiveName"), FPaths::GetCleanFilename(GetArchivePath(Target, FString())) );

			FString POFileName;
			// The output path for a specific culture is a file path.
			if (CultureName.IsSet() && OutputPathOverride.IsSet())
			{
				POFileName =  FPaths::GetCleanFilename( OutputPathOverride.GetValue() );
			}
			// Use the default PO file's name.
			else
			{
				POFileName = FPaths::GetCleanFilename( GetDefaultPOFileName( Target ) );
			}
			ConfigSection.Add( TEXT("PortableObjectName"), POFileName );
		}

		Script.Dirty = true;

		return Script;
	}

	FString GetImportScriptPath(const ULocalizationTarget* const Target, const TOptional<FString> CultureName)
	{
		const FString ConfigFileDirectory = GetScriptDirectory();
		FString ConfigFilePath;
		if (CultureName.IsSet())
		{
			ConfigFilePath = ConfigFileDirectory / FString::Printf( TEXT("%s_Import_%s.%s"), *Target->Settings.Name, *CultureName.GetValue(), TEXT("ini") );
		}
		else
		{
			ConfigFilePath = ConfigFileDirectory / FString::Printf( TEXT("%s_Import.%s"), *Target->Settings.Name, TEXT("ini") );
		}
		return ConfigFilePath;
	}

	FLocalizationConfigurationScript GenerateExportScript(const ULocalizationTarget* const Target, const TOptional<FString> CultureName, const TOptional<FString> OutputPathOverride)
	{
		FLocalizationConfigurationScript Script;

		const FString ContentDirRelativeToGameDir = MakePathRelativeToProjectDirectory(FPaths::GameContentDir());

		// GatherTextStep0 - InternationalizationExport
		{
			FConfigSection& ConfigSection = Script.GatherTextStep(0);

			// CommandletClass
			ConfigSection.Add( TEXT("CommandletClass"), TEXT("InternationalizationExport") );

			ConfigSection.Add( TEXT("bExportLoc"), TEXT("true") );

			const FString SourcePath = ContentDirRelativeToGameDir / TEXT("Localization") / Target->Settings.Name;
			ConfigSection.Add( TEXT("SourcePath"), SourcePath );

			FString DestinationPath;
			// Overriding output path changes the destination directory for the PO file.
			if (OutputPathOverride.IsSet())
			{
				// The output path for a specific culture is a file path.
				if (CultureName.IsSet())
				{
					DestinationPath = MakePathRelativeToProjectDirectory( FPaths::GetPath(OutputPathOverride.GetValue()) );
				}
				// Otherwise, it is a directory path.
				else
				{
					DestinationPath = MakePathRelativeToProjectDirectory( OutputPathOverride.GetValue() );
				}
			}
			// Use the default PO file's directory path.
			else
			{
				DestinationPath = ContentDirRelativeToGameDir / TEXT("Localization") / Target->Settings.Name;
			}
			ConfigSection.Add( TEXT("DestinationPath"), DestinationPath );

			TArray<const FCultureStatistics*> AllCultureStatistics;
			AllCultureStatistics.Add(&Target->Settings.NativeCultureStatistics);
			for (const FCultureStatistics& SupportedCultureStatistics : Target->Settings.SupportedCulturesStatistics)
			{
				AllCultureStatistics.Add(&SupportedCultureStatistics);
			}

			const auto& AddCultureToGenerate = [&](const int32 Index)
			{
				ConfigSection.Add( TEXT("CulturesToGenerate"), AllCultureStatistics[Index]->CultureName );
			};

			// Export for a specific culture.
			if (CultureName.IsSet())
			{
				const int32 CultureIndex = AllCultureStatistics.IndexOfByPredicate([CultureName](const FCultureStatistics* Culture) { return Culture->CultureName == CultureName.GetValue(); });
				AddCultureToGenerate(CultureIndex);
			}
			// Export for all cultures.
			else
			{
				for (int32 CultureIndex = 0; CultureIndex < AllCultureStatistics.Num(); ++CultureIndex)
				{
					AddCultureToGenerate(CultureIndex);
				}
			}

			// Do not use culture subdirectories if exporting a single culture to a specific directory.
			if (CultureName.IsSet() && OutputPathOverride.IsSet())
			{
				ConfigSection.Add( TEXT("bUseCultureDirectory"), "false" );
			}


			ConfigSection.Add( TEXT("ManifestName"), FPaths::GetCleanFilename(GetManifestPath(Target)) );
			ConfigSection.Add( TEXT("ArchiveName"), FPaths::GetCleanFilename(GetArchivePath(Target, FString())) );
			FString POFileName;
			// The output path for a specific culture is a file path.
			if (CultureName.IsSet() && OutputPathOverride.IsSet())
			{
				POFileName =  FPaths::GetCleanFilename( OutputPathOverride.GetValue() );
			}
			// Use the default PO file's name.
			else
			{
				POFileName = FPaths::GetCleanFilename( GetDefaultPOPath( Target, CultureName.Get( TEXT("") ) ) );
			}
			ConfigSection.Add( TEXT("PortableObjectName"), POFileName );
		}

		Script.Dirty = true;

		return Script;
	}

	FString GetExportScriptPath(const ULocalizationTarget* const Target, const TOptional<FString> CultureName)
	{
		const FString ConfigFileDirectory = GetScriptDirectory();
		FString ConfigFilePath;
		if (CultureName.IsSet())
		{
			ConfigFilePath = ConfigFileDirectory / FString::Printf( TEXT("%s_Export_%s.%s"), *Target->Settings.Name, *CultureName.GetValue(), TEXT("ini") );
		}
		else
		{
			ConfigFilePath = ConfigFileDirectory / FString::Printf( TEXT("%s_Export.%s"), *Target->Settings.Name, TEXT("ini") );
		}
		return ConfigFilePath;
	}

	FLocalizationConfigurationScript GenerateReportScript(const ULocalizationTarget* const Target)
	{
		FLocalizationConfigurationScript Script;

		const FString ContentDirRelativeToGameDir = MakePathRelativeToProjectDirectory(FPaths::GameContentDir());

		// GatherTextStep0 - GenerateTextLocalizationReport
		{
			FConfigSection& ConfigSection = Script.GatherTextStep(0);

			// CommandletClass
			ConfigSection.Add( TEXT("CommandletClass"), TEXT("GenerateTextLocalizationReport") );

			const FString SourcePath = ContentDirRelativeToGameDir / TEXT("Localization") / Target->Settings.Name;
			ConfigSection.Add( TEXT("SourcePath"), SourcePath );
			const FString DestinationPath = ContentDirRelativeToGameDir / TEXT("Localization") / Target->Settings.Name;
			ConfigSection.Add( TEXT("DestinationPath"), DestinationPath );

			ConfigSection.Add( TEXT("ManifestName"), FString::Printf( TEXT("%s.%s"), *Target->Settings.Name, TEXT("manifest") ) );

			for (const FCultureStatistics& CultureStatistics : Target->Settings.SupportedCulturesStatistics)
			{
				ConfigSection.Add( TEXT("CulturesToGenerate"), CultureStatistics.CultureName );
			}

			ConfigSection.Add( TEXT("bWordCountReport"), TEXT("true") );

			ConfigSection.Add( TEXT("WordCountReportName"), FPaths::GetCleanFilename( GetWordCountCSVPath(Target) ) );

			ConfigSection.Add( TEXT("bConflictReport"), TEXT("true") );

			ConfigSection.Add( TEXT("ConflictReportName"), FPaths::GetCleanFilename( GetConflictReportPath(Target) ) );
		}

		Script.Dirty = true;

		return Script;
	}

	FString GetReportScriptPath(const ULocalizationTarget* const Target)
	{
		return GetScriptDirectory() / FString::Printf( TEXT("%s_GenerateReports.%s"), *(Target->Settings.Name), TEXT("ini") );
	}

	FLocalizationConfigurationScript GenerateCompileScript(const ULocalizationTarget* const Target)
	{
		FLocalizationConfigurationScript Script;

		const FString ContentDirRelativeToGameDir = MakePathRelativeToProjectDirectory(FPaths::GameContentDir());

		// GatherTextStep0 - GenerateTextLocalizationResource
		{
			FConfigSection& ConfigSection = Script.GatherTextStep(0);

			// CommandletClass
			ConfigSection.Add( TEXT("CommandletClass"), TEXT("GenerateTextLocalizationResource") );

			const FString SourcePath = ContentDirRelativeToGameDir / TEXT("Localization") / Target->Settings.Name;
			ConfigSection.Add( TEXT("SourcePath"), SourcePath );
			const FString DestinationPath = ContentDirRelativeToGameDir / TEXT("Localization") / Target->Settings.Name;
			ConfigSection.Add( TEXT("DestinationPath"), DestinationPath );

			ConfigSection.Add( TEXT("ManifestName"), FString::Printf( TEXT("%s.%s"), *Target->Settings.Name, TEXT("manifest") ) );
			ConfigSection.Add( TEXT("ResourceName"), FString::Printf( TEXT("%s.%s"), *Target->Settings.Name, TEXT("locres") ) );

			TArray<const FCultureStatistics*> AllCultureStatistics;
			AllCultureStatistics.Add(&Target->Settings.NativeCultureStatistics);
			for (const FCultureStatistics& SupportedCultureStatistics : Target->Settings.SupportedCulturesStatistics)
			{
				AllCultureStatistics.Add(&SupportedCultureStatistics);
			}

			for (const FCultureStatistics* CultureStatistics : AllCultureStatistics)
			{
				ConfigSection.Add( TEXT("CulturesToGenerate"), CultureStatistics->CultureName );
			}
		}

		Script.Dirty = true;

		return Script;
	}

	FString GetCompileScriptPath(const ULocalizationTarget* const Target)
	{
		return GetScriptDirectory() / FString::Printf( TEXT("%s_Compile.%s"), *(Target->Settings.Name), TEXT("ini") );
	}
}