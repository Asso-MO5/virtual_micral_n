Contributing to the Project
===========================

Contribution to the project are accepted by way of Merge Requests. They can be on code,
on the build system/environment, on tests, on documentation, on whatever is
present in the depot.

Merge Requests must pass the checks of the build system and pass a review.

If you have a code contribution, look at the [BUILD](BUILD.md) documentation to set up
your development environment. You should also look for information about the
general [Principles and Architecture](documentation/Architecture.md) file.

Contributors are referenced in the [CONTRIBUTORS](CONTRIBUTORS.md) file, in the case the
repository is separated from its history.

Also, look at the [LICENSE](LICENSE.md) and check if you abid with it.

Contributing flow
-----------------

The contribution goes through a Git Merge Request in the hosting system you found
the project in.

The branch is rebased as close as HEAD from the main branch as possible, ideally,
it should be auto-mergeable (prefer small Merge Requests).

The merge request is presented in a named branch with explanation of the objectives
of the change. The merge request must have an objective which brings value.

The merge request must pass the CI Checks.

The merge request is then reviewed by the maintainer and will be accepted if passing the review.

Coding Conventions
------------------

The project uses C++20.

Care must be taken about the dependencies. Libraries already form a first level
of dependency tree that you can check in the [dependencies](documentation/Dependencies.md)
file (yet to be written).

Submitted C++ code files must be formatted with the project .clang-format. There's
no automatic check on it yet (but that could be a nice contribution to CI).

Cppcheck (ANALYZE_CPPCHECK target on CMake) must not add warnings.

Clang-Tidy is on its way.

Prefer high level explanation on classes over line by line paraphrase in comments.
If it can be explained by naming a variable or a function, don't use comments.

Prefer positiveness in your naming, to protect from double (or worse) negations.

Respect the coding style you see around you.

If unsure, refer to [CppCoreGuidelines](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md).
Note that not all code is strictly compliant to them, but will have a general tendency
toward them.
