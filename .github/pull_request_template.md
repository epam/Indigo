
## Remove-me-section
* Notify reviewers about the pull request
* Keep only necessary sections below for the review

## Generic request
- [ ] PR name follows the pattern `#1234 – issue name`
- [ ] branch name does not contain '#'
- [ ] base branch (master or release/xx) is correct
- [ ] PR is linked with the issue
- [ ] task status changed to "Code review"
- [ ] code follows product standards
- [ ] regression tests updated
### For release/xx branch
- [ ] backmerge to master (or newer release/xx) branch is created
### Optional
- [ ] unit-tests written
- [ ] documentation updated

## Backmerge request
- [ ] PR name follows the pattern `Backmerge: #1234 – issue name`
- [ ] PR is linked with the issue
- [ ] base branch (master or release/xx) is correct
- [ ] code contains only backmerge changes

## Bump request
- [ ] PR name follows the pattern `Bump version to ...`
- [ ] add brackets \[ \] for 'skip ci' and put it into the body

