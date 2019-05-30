#!/usr/bin/env bash

set -e
scp -o StrictHostKeyChecking=no -r c/docs "$COMPANY_SRV":"$COMPANY_SRV_SUPPORT_DIR/c"
scp -o StrictHostKeyChecking=no -r c/lib "$COMPANY_SRV":"$COMPANY_SRV_SUPPORT_DIR/c"
scp -o StrictHostKeyChecking=no -r c/utils "$COMPANY_SRV":"$COMPANY_SRV_SUPPORT_DIR/c"
scp -o StrictHostKeyChecking=no -r c/examples "$COMPANY_SRV":"$COMPANY_SRV_SUPPORT_DIR/c"

scp -o StrictHostKeyChecking=no -r python/docs "$COMPANY_SRV":"$COMPANY_SRV_SUPPORT_DIR/python"
