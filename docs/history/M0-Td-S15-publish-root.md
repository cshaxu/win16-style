# M0 Td S15: future publication destination

The user directs future binary releases to the project assets root and `O:/winnt` root, replacing the previous `O:/winnt/system32` destination. Execution rules, the project goal and the package publication script use the new fixed path. Historical receipts remain factual and are not rewritten. This governance change preserves the active M0 T18 S64 packet and does not create a numerical T or change product code, build inputs or ABI.

The updated publisher retains the four-file allowlist, tested-package hash checks, destination/reparse validation, backups and post-copy hash verification. No package is published by this governance-only change. Before changes, all four existing `O:/winnt` files already matched the corresponding current assets SHA256 values; no existing files were replaced or removed.

Validation: publisher ValidateOnly passed against the tested four-file package and both destinations; source-manifest self-test and governance self-test passed (including 13 rejected cases). The existing destination files matched the assets SHA256 values.
