To build the `pcp-pmda-qpid` RPMS:

1. Configure your local build environment, if not already:
  ```bash
  mkdir -p ~/rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
  [ -e ~/.rpmmacros ] || echo '%_topdir %(echo $HOME)/rpmbuild' > ~/.rpmmacros
  ```

2. Install the pre-requisites, if not already:
  ```bash
  sudo yum install boost cmake pcp-libs pcp-libs-devel [pcp-pmda-cpp-devel] \
      qpid-cpp-client qpid-cpp-client-devel qpid-qmf qpid-qmf-devel
  ```
  * Note, `pcp-pmda-cpp-devel` is not a standard Fedora / Red Hat package.  If
    its not already within your own yum repos, you can build it spearately via
    [these instructions](https://github.com/pcolby/pcp-pmda-cpp/tree/master/package/rpm).

3. Download the source archive:
  ```bash
  wget --content-disposition --directory-prefix=~/rpmbuild/SOURCES \
      https://github.com/pcolby/pcp-pmda-qpid/releases/tag/v<VERSION>
  ```

4. Download the spec file:
  ```bash
  wget --directory-prefix=~/rpmbuild/SPECS \
      https://raw2.github.com/pcolby/pcp-pmda-qpid/master/package/rpm/pcp-pmda-qpid.spec

  ```

5. Build the RPMs:
  ```bash
  rpmbuild -ba ~/rpmbuild/SPECS/pcp-pmda-qpid.spec
  ```

This will produce the following RPMs:
* `pcp-pmda-qpid-debuginfo` - debug info for the `pcp-pmda-qpid-*` packages.
* `pcp-pmda-qpid-qmf1` - QMFv1 version of the Qpid PMDA.
