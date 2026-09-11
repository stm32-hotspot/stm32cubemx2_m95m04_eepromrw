/**
  ******************************************************************************
  * file           : example.c
  * brief          : example program body
  ******************************************************************************
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "example.h"
#include "m95m04.h"
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define LOCK_ID_PAGE_EXECUTE 0U /* Set to 1 to execute the permanent ID page lock sequence */

#define SIZE100         100U   /* Size used for small test chunks                        */
#define SIZE200         200U   /* Size used for page-related test buffers                */
#define SIZE1024        1024U  /* Size used for full-page data test on main array        */

/* Private variables ---------------------------------------------------------*/
m95m04_object_t *pM95m040;     /* Pointer to the M95M04 driver object instance           */
uint8_t ReceiveBuff[SIZE1024] = {0};  /* General-purpose receive buffer for data reads   */
uint8_t ReadBuff[SIZE200] = {0};      /* Local buffer reserved for page tests            */
uint8_t TransmitBuff[SIZE1024] = {0}; /* General-purpose transmit buffer for data writes*/
uint8_t SampleData_Test_Page[SIZE200] = {0}; /* Reserved for specific page test patterns */
/**
  * ########## Step 1 ##########
  * The init of M95M04 is triggered by the application code
  */
app_status_t app_init(void)
{
  app_status_t return_status = EXEC_STATUS_ERROR;
  /* Get M95M04 object from BSP/driver layer */
  pM95m040 = MX_M95M04_getobject();
  /* Initialize M95M04 device instance 0 */
  if (m95m04_drv_init(pM95m040, MX_M95M04) != 0)
  {
    PRINTF("[ERROR] Step 1: M95M04 EEPROM init error\r\n");
    goto _app_init_exit;
  }
  PRINTF("[INFO] Step 1: M95M04 EEPROM init completed\r\n");

  /* Initialization completed successfully */
  return_status = EXEC_STATUS_INIT_OK;

_app_init_exit:
  return return_status;
}

/**
  * ########## Step 2 ##########
  * Perform Read and Write operations.
  * The values are displayed on the terminal.
  * output: EXEC_STATUS_OK if OK, EXEC_STATUS_ERROR in case of error
  */
app_status_t app_process(void)
{
  app_status_t return_status = EXEC_STATUS_OK;

  /* Test write-enable and write-disable commands and status register updates */
  app_status_t return_status_ed = M95M04_TestWriteEnableDisable();

  /* Test single-byte write/read sequence */
  app_status_t return_status_tb = M95M04_TestSingleByte();

  /* Test multi-byte data write/read and erase sequence */
  app_status_t return_status_td = M95M04_TestData();

#if (M95M04_ID_PAGE_ENABLE == 1)
  /* Exercise ID page read/write operations */
  app_status_t return_status_tid = M95M04_TestIDPage();

  /* Read and display the current ID page lock status */
  app_status_t return_status_tidls = M95M04_IDPageLockStatus();

  /* Optional permanent ID page lock (requires LOCK_ID_PAGE_EXECUTE set to 1) */
  app_status_t return_status_tidl = M95M04_IDPageLock();
#else
  /* ID page feature not compiled in; treat as successful for global test result */
  app_status_t return_status_tid = EXEC_STATUS_OK;
  app_status_t return_status_tidls = EXEC_STATUS_OK;
  app_status_t return_status_tidl = EXEC_STATUS_OK;
#endif /* (M95M04_ID_PAGE_ENABLE == 1) */

  /* Aggregate outcome from all test routines */
  if ((return_status_ed == EXEC_STATUS_OK)
      && (return_status_tb == EXEC_STATUS_OK)
      && (return_status_td == EXEC_STATUS_OK)
      && (return_status_tid == EXEC_STATUS_OK)
      && (return_status_tidls == EXEC_STATUS_OK)
      && (return_status_tidl == EXEC_STATUS_OK))
  {
    PRINTF("\n\nAll test cases PASSED.\r\n");
    return_status = EXEC_STATUS_OK;
  }
  else
  {
    PRINTF("\n\nTest cases FAILED.\r\n");
    return_status = EXEC_STATUS_ERROR;
  }

  return return_status;
}

/** ########## Step 3 ##########
  * In this example, app_deinit is never called and is provided as a reference only.
  */
app_status_t app_deinit(void)
{
  /* Deinitialize M95M04 driver and free associated hardware resources */
  if (m95m04_drv_deinit(pM95m040) != 0)
  {
    PRINTF("[ERROR] Step 3: EEPROM deinit error\r\n");
    return EXEC_STATUS_ERROR;
  }

  return EXEC_STATUS_OK;
}

/**
  * @brief  Tests Write enable and write disable functionality on M95M04.
  *         Reads status register before and after each command and prints values.
  * @param  None
  * @retval app_status_t
  */
app_status_t M95M04_TestWriteEnableDisable(void)
{
  PRINTF("[INFO] Test 1: READ STATUS REGISTER\r\n");
  uint8_t regval = 0xFF; /* Local variable holding status register content */

  /* Initial read of status register */
  if (m95m04_drv_read_status_register(pM95m040, &regval) != 0)
  {
    PRINTF("Read Status Register ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }

  PRINTF("[INFO] STATUS REGISTER : 0x%x\r\n", regval);

  /* Issue WRITE ENABLE command */
  if (m95m04_drv_write_enable(pM95m040) != 0)
  {
    PRINTF("Write Enable ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }

  PRINTF("[INFO] WRITE ENABLE\r\n");

  /* Read back status register to see WRITE ENABLE bit effect */
  if (m95m04_drv_read_status_register(pM95m040, &regval) != 0)
  {
    PRINTF("Read Status Register ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }
  PRINTF("[INFO] STATUS REGISTER : 0x%x\r\n", regval);

  /* Issue WRITE DISABLE command */
  if (m95m04_drv_write_disable(pM95m040) != 0)
  {
    PRINTF("Write Disable ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }
  PRINTF("[INFO] WRITE DISABLE\r\n");

  /* Final status register read after write disable */
  if (m95m04_drv_read_status_register(pM95m040, &regval) != 0)
  {
    PRINTF("Read Status Register ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }
  PRINTF("[INFO] STATUS REGISTER : 0x%x\r\n", regval);

  return EXEC_STATUS_OK;
}

/**
  * @brief  Test single-byte write and read on M95M04.
  *         Programs one byte, verifies it, then restores its content to 0xFF.
  * @param  None
  * @retval app_status_t
  */
app_status_t M95M04_TestSingleByte(void)
{
  app_status_t ret_val = EXEC_STATUS_OK;

  PRINTF("\n\n***************************************************************\r\n");
  PRINTF("                   -- SPI M95M04 EEPROM TEST SINGLE BYTE-- \r\n");
  PRINTF("***************************************************************\r\n");

  uint8_t tx = 0x89;                 /* Test byte to program                                     */
  uint8_t rx = 0xFF;                 /* Byte read from memory                                    */
  unsigned int target_addr = 0x00;   /* Address used for single-byte test                        */

  /* Read current data at target address before modification */
  if (m95m04_drv_read_byte(pM95m040, &rx, target_addr) == 0)
  {
    PRINTF("Read Memory Data : 0x%x at Address : 0x%x\r\n", rx, target_addr);
  }

  /* Enable write operations before programming */
  if (m95m04_drv_write_enable(pM95m040) != 0)
  {
    PRINTF("Write Enable ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }

  /* Program one byte at target address */
  int32_t w_ret = m95m04_drv_write_byte(pM95m040, &tx, target_addr);

  /* Read back the programmed byte */
  int32_t r_ret = m95m04_drv_read_byte(pM95m040, &rx, target_addr);

  if ((w_ret == 0) && (r_ret == 0))
  {
    if (rx == tx)
    {
      PRINTF("TestByte | Target: %s | Address: 0x%u | TX: 0x%x | RX: 0x%x | Result: PASSED \r\n",
             "M95M04", target_addr, tx, rx);

      /* Clear location after test by programming 0xFF */
      if (m95m04_drv_write_enable(pM95m040) != 0)
      {
        PRINTF("Write Enable ERROR\r\n");
        return EXEC_STATUS_ERROR;
      }
      else
      {
        tx = 0xFF;
        m95m04_drv_write_byte(pM95m040, &tx, target_addr);
      }
    }
    else
    {
      PRINTF("TestByte | Target: %s| Address: %u | Result: FAILED \r\n", "M95M04", target_addr);
      ret_val = EXEC_STATUS_ERROR;
    }
  }
  else
  {
    PRINTF("TestByte | Target: %s| Write or Read Operation FAILED \r\n", "M95M04");
    ret_val = EXEC_STATUS_ERROR;
  }

  return ret_val;
}

/**
  * @brief  Test multi-byte data write/read on M95M04.
  *         Writes 1024 bytes from a pattern buffer, verifies them, then erases to 0xFF.
  * @param  None
  * @retval app_status_t
  */
app_status_t M95M04_TestData(void)
{

  app_status_t ret_val = EXEC_STATUS_OK;
  uint16_t idx;

  PRINTF("\n\n***************************************************************\r\n");
  PRINTF("                   -- SPI M95M04 EEPROM TEST DATA-- \r\n");
  PRINTF("***************************************************************\r\n");

  unsigned int target_addr = 0x10;        /* Start address for 1024-byte data test                  */
  memset(ReceiveBuff, 0x00, sizeof(ReceiveBuff));

  /* Display memory content before write */
  PRINTF("\n\nMemory contents before write (1024 bytes): \r\n");
  if (m95m04_drv_read_data(pM95m040, ReceiveBuff, target_addr, SIZE1024) == 0)
  {
    for (idx = 0; idx < SIZE1024; idx++)
    {
      PRINTF("0x%x ", ReceiveBuff[idx]);
    }
  }

  /* Prepare transmit buffer with fixed pattern 0xAB */
  memset(TransmitBuff, 0xAB, sizeof(TransmitBuff));
  int32_t w_ret = m95m04_drv_write_data(pM95m040, TransmitBuff, target_addr, SIZE1024);

  /* Clear receive buffer and perform readback */
  memset(ReceiveBuff, 0x00, sizeof(ReceiveBuff));
  int32_t r_ret = m95m04_drv_read_data(pM95m040, ReceiveBuff, target_addr, SIZE1024);

  if ((w_ret == 0) && (r_ret == 0))
  {
    PRINTF("\n\nMemory contents after write: \r\n");
    /* Compare written and read data byte-by-byte */
    for (idx = 0; idx < SIZE1024; idx++)
    {
      if (TransmitBuff[idx] == ReceiveBuff[idx])
      {
        PRINTF("0x%x ", ReceiveBuff[idx]);
      }
      else
      {
        break;
      }
    }

    if (idx == SIZE1024)
    {
      PRINTF("\nAll data to M95M04 written successfully!\r\n");
    }
    else
    {
      PRINTF("Error in M95M04 write.\r\n");
      return EXEC_STATUS_ERROR;
    }

    /* Clear the tested area to 0xFF to restore default state */
    PRINTF("\nReset memory to 0xFF from Address:0x%2.2X \r\n", target_addr);
    memset(TransmitBuff, 0xFF, sizeof(TransmitBuff));

    if (m95m04_drv_write_data(pM95m040, TransmitBuff, target_addr, SIZE1024) != 0)
    {
      ret_val = EXEC_STATUS_ERROR;
    }
    else
    {
      /* Read back after erase to confirm contents */
      memset(ReceiveBuff, 0x00, sizeof(ReceiveBuff));
      if (m95m04_drv_read_data(pM95m040, ReceiveBuff, target_addr, SIZE1024) == 0)
      {
        for (idx = 0; idx < SIZE1024; idx++)
        {
          PRINTF("0x%x ", ReceiveBuff[idx]);
        }
      }
      PRINTF("\nMemory contents of M95M04 cleared to 0xFF \r\n");
    }

  }
  else
  {
    PRINTF("M95M04 Test Memory Data: FAILED \r\n");
    ret_val = EXEC_STATUS_ERROR;
  }

  return ret_val;
}

#if (M95M04_ID_PAGE_ENABLE == 1)
/**
  * @brief  Test Identification Page read/write on M95M04.
  *         Reads, programs and then erases the ID page area.
  * @param  None
  * @retval app_status_t
  */
app_status_t  M95M04_TestIDPage(void)
{
  app_status_t ret_val = EXEC_STATUS_OK;
  uint16_t idx;

  PRINTF("\n\n***************************************************************\r\n");
  PRINTF("                   -- SPI M95M04 EEPROM TEST ID PAGE -- \r\n");
  PRINTF("***************************************************************\r\n");

  unsigned int target_addr = 0x00;        /* Start address for ID page access                        */
  memset(ReceiveBuff, 0x00, sizeof(ReceiveBuff));

  /* Dump ID page content before modification */
  PRINTF("\n\nMemory contents of ID Page before write (512 bytes): \r\n");
  if (m95m04_drv_read_id_page(pM95m040, ReceiveBuff, target_addr, SIZE512) == 0)
  {
    for (idx = 0; idx < SIZE512; idx++)
    {
      PRINTF("0x%x ", ReceiveBuff[idx]);
    }
  }

  /* Prepare ID page buffer with pattern 0x66 */
  memset(TransmitBuff, 0x66, sizeof(TransmitBuff));

  /* Write Enable to modify ID page */
  if (m95m04_drv_write_enable(pM95m040) != 0)
  {
    PRINTF("Write Enable ERROR\r\n");
    return EXEC_STATUS_ERROR;
  }

  /* Program ID page with pattern */
  int32_t w_ret = m95m04_drv_write_id_page(pM95m040, TransmitBuff, target_addr, SIZE512);

  /* Read back ID page after write */
  memset(ReceiveBuff, 0x00, sizeof(ReceiveBuff));
  int32_t r_ret = m95m04_drv_read_id_page(pM95m040, ReceiveBuff, target_addr, SIZE512);

  if ((w_ret == 0) && (r_ret == 0))
  {
    PRINTF("\n\nMemory contents after write: \r\n");
    for (idx = 0; idx < SIZE1024; idx++)
    {
      if (TransmitBuff[idx] == ReceiveBuff[idx])
      {
        PRINTF("0x%x ", ReceiveBuff[idx]);
      }
      else
      {
        break;
      }
    }

    if (idx == SIZE512)
    {
      PRINTF("\nAll data to M95M04 ID Page written successfully!\r\n");
    }
    else
    {
      PRINTF("Error in M95M04 write.\r\n");
      return EXEC_STATUS_ERROR;
    }

    /* Clear ID page back to 0xFF after the test */
    PRINTF("\nReset ID Page memory to 0xFF from Address:0x%2.2X \r\n", target_addr);
    memset(TransmitBuff, 0xFF, sizeof(TransmitBuff));

    /* Write Enable before clearing ID page */
    if (m95m04_drv_write_enable(pM95m040) != 0)
    {
      PRINTF("Write Enable ERROR\r\n");
      return EXEC_STATUS_ERROR;
    }

    if (m95m04_drv_write_id_page(pM95m040, TransmitBuff, target_addr, SIZE512) != 0)
    {
      ret_val = EXEC_STATUS_ERROR;
    }
    else
    {
      memset(ReceiveBuff, 0x00, sizeof(ReceiveBuff));
      if (m95m04_drv_read_id_page(pM95m040, ReceiveBuff, target_addr, SIZE512) == 0)
      {
        for (idx = 0; idx < SIZE512; idx++)
        {
          PRINTF("0x%x ", ReceiveBuff[idx]);
        }
      }
      PRINTF("\nID Page Memory contents of M95M04 cleared to 0xFF \r\n");
    }

  }
  else
  {
    PRINTF("M95M04 Test ID Page Memory Data: FAILED \r\n");
    ret_val = EXEC_STATUS_ERROR;
  }

  return ret_val;
}

/**
  * @brief  Read and display ID page lock status on M95M04.
  * @param  None
  * @retval app_status_t
  */
app_status_t M95M04_IDPageLockStatus(void)
{
  uint8_t lock_status;              /* Holds returned lock bit status value                     */
  app_status_t ret_val = EXEC_STATUS_OK;

  PRINTF("\n\n***************************************************************\r\n");
  PRINTF("             -- SPI M95M04 EEPROM ID PAGE LOCK STATUS -- \r\n");
  PRINTF("***************************************************************\r\n");

  /* Query ID page lock state from device */
  if (m95m04_drv_read_id_page_lock_status(pM95m040, &lock_status) != 0)
  {
    ret_val = EXEC_STATUS_ERROR;
  }
  else
  {
    if (lock_status == 0U)
    {
      PRINTF("\n\rID Page Lock not active\r\n");
    }
    else
    {
      PRINTF("\n\rID Page Lock active\r\n");
    }

  }

  return ret_val;
}

/**
  * @brief  Permanently lock ID page of M95M04 if enabled by compile-time switch.
  * @param  None
  * @retval app_status_t
  */
app_status_t M95M04_IDPageLock(void)
{

  app_status_t ret_val = EXEC_STATUS_UNKNOWN;

  PRINTF("\n\n***************************************************************\r\n");
  PRINTF("             -- SPI M95M04 EEPROM LOCK ID PAGE PERMANENTLY -- \r\n");
  PRINTF("***************************************************************\r\n");

#if (!LOCK_ID_PAGE_EXECUTE)
  /* Lock not executed because protection is disabled at compile time */
  PRINTF("[WARN] Enable Lock ID Page function in Application to execute! \r\n");
  ret_val = EXEC_STATUS_OK;
#else
  /* Request permanent ID page lock from driver */
  if (m95m04_drv_lock_id_page(pM95m040) != 0)
  {
    ret_val = EXEC_STATUS_ERROR;
  }
  else
  {
    ret_val = EXEC_STATUS_OK;
  }
#endif /* LOCK_ID_PAGE_EXECUTE */

  return ret_val;
}

#endif /* (M95M04_ID_PAGE_ENABLE == 1) */
