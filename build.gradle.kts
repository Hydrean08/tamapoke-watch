buildscript {
    repositories {
        google()
        mavenCentral()
    }
    configurations.classpath {
        resolutionStrategy {
            force(
                "org.jetbrains.kotlin:kotlin-reflect:2.1.0",
                "org.jetbrains.kotlin:kotlin-stdlib:2.1.0",
                "org.jetbrains.kotlin:kotlin-stdlib-jdk7:2.1.0",
                "org.jetbrains.kotlin:kotlin-stdlib-jdk8:2.1.0"
            )
        }
    }
    dependencies {
        classpath("com.android.tools.build:gradle:8.12.0")
    }
}
