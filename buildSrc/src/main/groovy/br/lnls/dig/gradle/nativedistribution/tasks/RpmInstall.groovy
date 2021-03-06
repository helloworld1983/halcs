package br.lnls.dig.gradle.nativedistribution.tasks

import org.gradle.api.distribution.Distribution
import org.gradle.api.tasks.Exec

import br.lnls.dig.gradle.nativedistribution.model.internal.RpmDistribution

public class RpmInstall extends Exec {
    private File rpmDirectory
    private RpmDistribution distribution

    public RpmInstall() {
        commandLine 'rpm', '-Uvh', '--force'
    }

    public void setRpmDirectory(File rpmDirectory) {
        this.rpmDirectory = rpmDirectory

        maybeSetRpmFileArgument()
    }

    public void setDistribution(Distribution distribution) {
        this.distribution = new RpmDistribution(distribution)

        dependsOn distribution.taskNameFor("distRpm")

        processDependencies()
        maybeSetRpmFileArgument()
    }

    private void processDependencies() {
        distribution.dependencies.each { dependency ->
            if (dependency.installTask != null)
                dependsOn dependency.installTask
        }
    }

    private void maybeSetRpmFileArgument() {
        if (distribution != null && rpmDirectory != null)
            args "$rpmDirectory.path/$distribution.outputFileName"
    }
}
