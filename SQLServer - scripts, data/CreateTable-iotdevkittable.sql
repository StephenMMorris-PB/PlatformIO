CREATE TABLE [dbo].[iotdevkittable]
(
 [Id] [bigint] IDENTITY(1,1) NOT NULL,
 [messageId] [varchar] (3) NULL,
 [temperature] [nvarchar](200) NULL,
 [humidity] [nvarchar](200) NULL,
 [PartitionId] [varchar] (3) NULL,
 [EventProcessedUtcTime] [datetime] NULL,
 [EventEnqueuedUtcTime] [datetime] NULL,
 [IoTHub] [nvarchar](200) NULL,
 CONSTRAINT [PK_iotdevkittable] PRIMARY KEY CLUSTERED
 (
  [id] ASC
 )WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]